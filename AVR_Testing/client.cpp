#include "client.h"
#include <QDataStream>

namespace AVR
{
    Client::Client(QObject* pwgt /*=0*/)    //Client constructor, initializing class members
        : QObject(pwgt),
          m_nNextBlockSize(0)
    {
        m_bConnected = false;
        m_pTcpSocket = nullptr;
        m_iTrueAVRPosition = 0;
        m_iMaxPos = 0;
    }

    Client::~Client()
    {
        Disconnect();   //Disconnect and clean-up
    }

    void Client::Connect(const QString& strHost, int nPort) //Connects client to AVR host
    {
        if (m_bConnected)
            Disconnect();   //Interrupt and clean-up current connection if it exists before creating new.

        emit SetConnectItemEnabled(false);  //Disable 'Connect' item in menu for safe work
        m_pTcpSocket = new QTcpSocket(this);    //Creating new socket
        m_bConnected = true;    //Changing connected state to true
        m_pTcpSocket->connectToHost(strHost, nPort);    //Connecting new socket to host

        //Connecting socket's signals with Client's clots (connected, ready to read and error occurred signals)
        QObject::connect(m_pTcpSocket, &QTcpSocket::connected, this, &Client::slotConnected);
        QObject::connect(m_pTcpSocket, &QTcpSocket::readyRead, this, &Client::slotReadyRead);
        QObject::connect(m_pTcpSocket,
            static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error),
            this, &Client::slotError);
    }

    void Client::slotReadyRead()    //This slot triggers by QTcpSocket::readyRead signal. Processes all incoming messages.
    {
        QDataStream in(m_pTcpSocket);   //Init socket data stream
        in.setVersion(QDataStream::Qt_5_3); //Sets version of QDataStream
        while(true) //Reading incoming data in loop
        {
            if (!m_nNextBlockSize)  //Break if no data to read
            {
                if (m_pTcpSocket->bytesAvailable() < qint64(sizeof(quint16)))
                    break;
                in >> m_nNextBlockSize;
            }
            if (m_pTcpSocket->bytesAvailable() < m_nNextBlockSize)
                break;

            QString str;
            in >> str;  //Write data from socket to string
            HandleServerMessage(str);   //Parse received data
            m_nNextBlockSize = 0;
        }
    }

    void Client::slotError(QAbstractSocket::SocketError err)    //When socket error occurred
    {
        QString strError;
        if (err == QAbstractSocket::HostNotFoundError)
            strError = "Client Error: The host was not found.";
        else if (err == QAbstractSocket::RemoteHostClosedError)
            strError = "Information: The remote connection was closed. Disconnected.";
        else if (err == QAbstractSocket::ConnectionRefusedError)
            strError = "Client Error: The connection was refused.";
        else
            strError = "Client Error: " + QString(m_pTcpSocket->errorString()); //Other socket errors
        emit WriteLineToLog(strError);  //Write error information to log
        Disconnect(false);  //Close connection and clean-up client data
    }

    void Client::slotSendToServer(MessageType msg, int steps)   //Sends message to AVR host
    {
        QString FullMessage;
        if (msg == MessageType::MoveForNSteps)          //Write step quantity into message string
            FullMessage.sprintf("%i:%i", msg, steps);   //if going to send MoveForNSteps message
        else
            FullMessage.sprintf("%i", steps);   //Otherwise just write the message code.

        //Preparing message for sending through socket
        QByteArray arrBlock;
        QDataStream out(&arrBlock, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_2);
        out << quint16(0) << FullMessage;   //Stream our message to byte array block
        out.device()->seek(0);
        out << quint16(arrBlock.size() - sizeof(quint16));
        m_pTcpSocket->write(arrBlock);  //Write prepared data to socket
    }

    void Client::slotConnected()    //When socket connected
    {
        emit WriteLineToLog("Connection established successfully!");    //Write to log about it
        emit SetDisconnectItemEnabled(true);                            //Allow user to disconnect after connection
    }

    void Client::Disconnect(bool writeToLog)    //Disconnect from host and clean-up client data
    {
        if (m_bConnected)   //Do not clean-up if disconnected already
        {
            if (writeToLog)
                emit WriteLineToLog("Disconnecting from AVR.");
            m_pTcpSocket->close();  //Closing socket
            m_pTcpSocket->deleteLater();    //Asking him for self-delete
            //Nulling client data
            m_pTcpSocket = nullptr;
            m_bConnected = false;
            m_iTrueAVRPosition = 0;
            m_iMaxPos = 0;
        }
        //Blocking controls and allow user to connect again
        emit SetConnectItemEnabled(true);
        emit SetDisconnectItemEnabled(false);
        emit SetAVRControlsEnabled(false);
    }

    bool Client::IsConnected() const
    {
        return m_bConnected;    //Returns connected state
    }

    void Client::HandleServerMessage(const QString& message)    //Parses messages from AVR host
    {
        QString str, tmp;
        str = message;      //Save it
        int pos, delimiterPos;
        if (str[0] != '\\') //Check if special AVR message token exists (\p, \r, \m, etc.)
        {
             emit WriteLineToLog("Unknown server message: " + str);
             return;   //Do nothing if unknown message was received.
        }

        switch(str[1].toLatin1())   //Parsing message token
        {
            case 'p':   // "\p" token means AVR saying it's current position.
                        // this position may be untrue with some random probability.
                        // but if position is 0 - it's always return true position.

                pos = str.right(str.length() - 2).toInt();  //Position number comes after \p , saving it to pos
                emit WriteLineToLog("----------------------------");
                str.sprintf("AVR: Current position: %i", pos);  //Saying received position
                emit WriteLineToLog(str);
                if(pos == m_iTrueAVRPosition)   //Comparing localy calculated AVR position with received
                    emit WriteLineToLog("This position is true.");  //If they equal the position is true.
                else
                {   //If not - AVR is lying.
                    str.sprintf("AVR is lying! Position must be: %i", m_iTrueAVRPosition);
                    emit WriteLineToLog(str);
                }
                emit WriteLineToLog("----------------------------");
                break;

            case 'm':   // "\m" token means text message. It writes to log everything after \m in received message.
                str = str.right(str.length() - 2);
                emit WriteLineToLog(str);
                break;

            case 's':   // "\s" token means AVR reporting about successfuly finished move operation.
                emit WriteLineToLog("AVR: Success! Moving has been complete.");
                break;

            case 'i':   // "\i" token means AVR initializing client data when it was connected Client entity (not user) must to know
                        // current position and maximum position value. Message with this token comes instantly
                        // after client connects to AVR host. Position sent with this token is ALWAYS true.
                        // Example of message:      \i200:15000    It means current position is 200 and max is 15000.

                str = str.right(str.length() - 2);  //Remove token from string
                delimiterPos = str.indexOf(":", 0); //Find ':' delimiter pos
                tmp = str.left(delimiterPos);       //Save all string before delimiter (this is current pos)
                m_iTrueAVRPosition = tmp.toInt();   //Save true AVR position into client member variable.
                tmp = str.right(str.length() - (delimiterPos + 1)); //Get max pos
                m_iMaxPos = tmp.toInt();            //Save it too

                //Report about it
                str.sprintf("AVR: Current position is %i. Max position is %i.", m_iTrueAVRPosition, m_iMaxPos);

                //Now we know initial position of AVR system and maximum threshold of steps.
                //Client is ready, unlocking AVR controls for user.
                emit WriteLineToLog(str);
                emit WriteLineToLog("AVR: Ready for work.");
                emit SetAVRControlsEnabled(true);
                break;

            case 'r': // "\r" means AVR says it received client's message and it's going to execute it.
                      // E.g. we requested to move for certain quantity of steps and when AVR receives
                      // this message it says back that it's goind to do it and says how much steps client requested.
                      // Message example:      \r1:56    This means that we requested to move for some steps (code 1)
                      //                                 and quantity of steps is 56.
                      // other messages does not have any second value.

                str = str.right(str.length() - 2);  //Removing token from string
                if(str[0] == '1')   //Code 1 means callback for AVR::MessageType::MoveForNSteps request
                {
                    if(str[1] == ':' && str[2] != '\0')
                    {
                        pos = str.right(str.length() - 2).toInt();  //Saving received position

                        //Predicting will AVR move for such quantity of steps or not.
                        if(m_iTrueAVRPosition + pos <= m_iMaxPos && m_iTrueAVRPosition + pos > 0)
                            m_iTrueAVRPosition += pos; //If it will - add it to local AVR position.

                        //Saying what AVR is going to do.
                        str.sprintf("AVR: Received new order. Moving for %i steps...", pos);
                        emit WriteLineToLog(str);
                    }
                    else
                        emit WriteLineToLog("AVR: Received new order. Moving for unknown steps...");
                }
                else if(str[0] == '2') //Code 2 means callback for AVR::MessageType::MoveToZero request
                {
                    m_iTrueAVRPosition = 0; //So position is going to be nulled. Seting local position to 0.
                    emit WriteLineToLog("AVR: Received new order. Moving to zero...");
                }
                else if(str[0] == '3') //Code 3 means AVR going to tell us it's position.
                    emit WriteLineToLog("AVR: Received new order. Returning current position...");
                else    //Undefined behavior
                    emit WriteLineToLog("AVR: Received unknown order. Doing nothing.");
            break;

            default:    //If unknown token was received from server.
                str.sprintf("Unknown responce token '\\%c'", str[1].toLatin1());
                emit WriteLineToLog(str);
        }
    }
}
