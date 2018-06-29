#include "avrserver.h"
#include <QMessageBox>

/*
    Server messages for it's client always must contain special token at the begining.
    Messages without token wouldn't be understood by client.
    Token begins with '\' char + some letter which describes meaning of next message.


    There are such tokens used in this server implementation:

    \i - means AVR initializing client data when it was connected. Client entity (not user) must to know
         current position and maximum position value. Message with this token comes instantly
         after client connects to AVR host. Position sent with this token is ALWAYS true.
         Example of message:      \i200:15000    It means current position is 200 and max is 15000.

         Format:     \i<CurrentPosition>:<MaxPosition>


    \m - means text message. After this token comes any text message.

         Format:    \m<AnyText>


    \p - means AVR saying it's current position by request AVR::Message::Type::GetPosition
         This position may be untrue with some random probability.
         But if position is 0 - it's always return true position.

         Format:    \p<PositionNumber>


    \r - means AVR says it received client's message and it's going to execute it.
         E.g. we requested to move for certain quantity of steps and when AVR receives
         this message it says back that it's goind to do it and says how much steps client requested.
         Message example:      \r1:56    This means that we requested to move for some steps (code 1)
                                         and quantity of steps is 56.
         Other messages does not have any second value.

         Format:    \r<MessageActionCode><StepCount>   or   \r<MessageActionCode>

         Available message action codes:
             1 - equals AVR::Message::Type::MoveForNSteps
             2 - equals AVR::Message::Type::MoveToZero
             3 - equals AVR::Message::Type::GetPosition


    \s - means AVR reporting about successfuly finished move operation. Does not contain anything after token.
         All data after \s will be ignored. It just notifies client about successfuly finished moving.

         Format:    \s
*/

AVR::Server::Server(const QHostAddress& host, int nPort, QObject* pwgt /*=0*/) : QObject(pwgt)
, m_nNextBlockSize(0)
{
    m_ptcpServer = new QTcpServer(this);    //Create new server instance
    if (!m_ptcpServer->listen(host, nPort)) //Starting listening port for certain host
    {
        //Well, server listen has been failed...
        //Show error message and stop the server
        QMessageBox::critical(0,"Server Error","Unable to start the server: " + m_ptcpServer->errorString());
        m_ptcpServer->close();
        throw Exeption::ListenFailed;   //Throw an exeption, server listen has been failed
        return;   //Interrupt server init
    }
    //Connect new connection signal with server's slot
    QObject::connect(m_ptcpServer, &QTcpServer::newConnection, this, &Server::slotNewConnection);
    m_theOnlyClient = nullptr;
    m_bHasClient = false;
}

AVR::Server::~Server()
{
    //Stop server, clean-up data
    if(m_bHasClient)
        m_theOnlyClient->close();
    m_theOnlyClient->deleteLater();
    m_ptcpServer->close();
    delete m_ptcpServer;
}

void AVR::Server::slotNewConnection()   //When new client connected
{
    if(m_bHasClient)    //If we already have a client...
    {
        //...kick him away and say him that he cannot connect now
        auto newClient = m_ptcpServer->nextPendingConnection();
        sendToClient(newClient, "\\mAVR System already has a client. Connection denied.");
        newClient->close();
    }
    else    //If no client, server will accept new connection
    {
        //Save pointer to socket of new client and connect server's slot with it's read and disconnect signals
        m_theOnlyClient = m_ptcpServer->nextPendingConnection();
        QObject::connect(m_theOnlyClient, &QTcpSocket::disconnected, this, &AVR::Server::OnClientDisconnected);
        QObject::connect(m_theOnlyClient, &QTcpSocket::readyRead, this, &Server::slotReadClient);
        //Say client that he has been connected successfuly.
        sendToClient(m_theOnlyClient, "\\mAVR Response: Connected successfuly!");
        m_bHasClient = true;    //Now we have a client
        emit ChangeConnectionLabel(true);   //Say UI to change connected lable state to Connected
        emit AskForClientInit();    //Ask AVR System to say server it's current position and max position
                                    //for sending it to client for it's initialization.
    }
}

void AVR::Server::slotReadClient()  //Read data when client sends to AVR something
{
    //Get sender
    QTcpSocket* pClientSocket = (QTcpSocket*)sender();
    QDataStream in(pClientSocket);  //Create data stream for sender's socket
    in.setVersion(QDataStream::Qt_5_3); //Set version of data stream
    while (true)    //Reading loop
    {
        if (!m_nNextBlockSize)  //Break loop if nothing to read
        {
            if (pClientSocket->bytesAvailable() < qint64(sizeof(quint16)))
                break;
            in >> m_nNextBlockSize;
        }
        if (pClientSocket->bytesAvailable() < m_nNextBlockSize)
            break;
        QString incomingData;
        in >> incomingData; //Save data to string
        m_nNextBlockSize =0;
        //Received data now in format <ActionCode>:<StepCount>
        //Forming AVR::Message instance
        AVR::Message avrMsg = FormMessage(incomingData);
        //Sending it to AVR System message queue
        emit AVRMessage(avrMsg);
    }
}

void AVR::Server::sendToClient(QTcpSocket* pSocket, const QString& str) //Sends data to client
{
    //Send string to client's socket by block array through data stream
    QByteArray arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_3);
    out << quint16(0) << str;
    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));
    pSocket->write(arrBlock);   //Writing block array to socket
}

AVR::Message AVR::Server::FormMessage(const QString& str)   //Create AVR::Message from incoming client's message
{
    int delimiterPos = str.indexOf(":", 0); //Finding ':' delimiter position
    if(delimiterPos != -1)
    {
        QString tmp;
        int msg, pos;
        tmp = str.left(delimiterPos);
        msg = tmp.toInt();  //Saving message code
        tmp = str.right(str.length() - (delimiterPos + 1));
        pos = tmp.toInt();  //Saving position
        return AVR::Message(AVR::Message::Type(msg), pos);  //Returning AVR::Message
    }
    else    //Iff not found - just converting message to int and passing it to AVR::Message as message type
        return AVR::Message(AVR::Message::Type(str.toInt()));
}

void AVR::Server::AVRWorkIsComplete()   //When AVR finished it's work send client success message
{
    if(m_bHasClient)
        sendToClient(m_theOnlyClient, "\\s");   //Success token
}

void AVR::Server::OnAVRError(AVRSystem::Exeption code)  //When AVR error occured
{
    QString errormsg = "\\mAVR Error: ";    //Message token and message text

    switch(code)
    {
        case AVRSystem::Exeption::UnknownMessage:
            errormsg += "Unknown type of incoming message.";
            break;
        case AVRSystem::Exeption::ValueIsLowerThanZero:
            errormsg += "Requested position is lower than 0.";
            break;
        case AVRSystem::Exeption::TooHighValue:
            errormsg += "Requested position is too large and exceeds the maximum value.";
            break;
        case AVRSystem::Exeption::AlreadyMoving:
            errormsg += "Unexpected behavior. Attempting to move while AVR already moving. Operation canceled.";
            break;
        default:
            errormsg += "Unknown error occured.";
    }

    if(m_bHasClient)
        sendToClient(m_theOnlyClient, errormsg);    //Sending error message to client if it exists
}

void AVR::Server::SendPosition(int pos) //Sending AVR position to client
{
    QString msg;
    msg.sprintf("\\p%i", pos);  //Position token and received position from AVR System
    if(m_bHasClient)
        sendToClient(m_theOnlyClient, msg); //Sending position to client if it exists
}

void AVR::Server::OnClientDisconnected()    //This slot runs when client has been disconnected
{
    m_bHasClient = false;   //We have no client
    m_theOnlyClient = nullptr;
    QTcpSocket* clientSocket = qobject_cast<QTcpSocket *>(QObject::sender()); //Getting disconnected client's socket
    clientSocket->deleteLater();    //Asking him for deleting
    emit ChangeConnectionLabel(false);  //Sending signal to UI: Change connection lable state to Disconnected
}

//When AVR System received a message from client
void AVR::Server::OnMessageReceived(Message::Type type, int ReceivedSteps)
{
    QString msg = "\\r";    //Message received token
    switch(int(type))
    {
        case Message::Type::MoveForNSteps:  //If MoveForNSteps - say client how much steps AVR will move.
            msg = "";
            msg.sprintf("\\r1:%i", ReceivedSteps);
            break;
        //Just writing message type
        case Message::Type::MoveToZero:
            msg += "2";
            break;

        case Message::Type::GetPosition:
            msg += "3";
            break;
    }

    if(m_bHasClient)
        sendToClient(m_theOnlyClient, msg); //Sending response to client if it exists
}

//When AVR Systems sends init data to server for new client
void AVR::Server::OnClientInit(int currentPos, int maxPos)
{
    QString msg;
    msg.sprintf("\\i%i:%i", currentPos, maxPos);    //Form init data message with init token
    if(m_bHasClient)
        sendToClient(m_theOnlyClient, msg); //Sending init data to new client if it still exists
}
