#pragma once

#include <QObject>
#include <QTcpSocket>

namespace AVR
{
    enum MessageType    //All possible messages
    {
        Unknown,
        MoveForNSteps,
        MoveToZero,
        GetPosition,
        TYPE_MAX
    };

    // AVR Client class. This class works with AVR connection, sends, receives and parses it's messages.
    class Client : public QObject
    {
        Q_OBJECT

     private:
        QTcpSocket* m_pTcpSocket;   //Connection socket
        quint16 m_nNextBlockSize;   //Socket's next block size
        bool m_bConnected;          //Connection state of client (connected or not)
        int m_iTrueAVRPosition;     //Position of AVR. Calculated by client. Initialy requested from server.
        int m_iMaxPos;              //Maximum available AVR position. Initialy requested from server.

        void HandleServerMessage(const QString& message);   //Method for parsing incoming messages from server.

    public:
        Client(QObject* pwgt = 0);
        ~Client();

        void Connect(const QString& strHost, int nPort);    //Connects client to AVR host
        void Disconnect(bool writeToLog = true);            //Disconnect from AVR host. If writeToLog == false it wouldn't log this.
        bool IsConnected() const;   //Checks current connection state

    private slots:
        void slotReadyRead();       //This slot triggers by QTcpSocket::readyRead signal. Processes all incoming messages.
        void slotError(QAbstractSocket::SocketError err);   //Triggers when any error happens on QTcpSocket.
        void slotConnected();      //Triggered when connected to AVR host.

    public slots:
        void slotSendToServer(MessageType msg, int steps);    //Sends action message to AVR and quantity of steps if needed.

    signals:
        void WriteLineToLog(const QString& text);   //Writes new line directly to textEdit widget on main form

        //This sets enabled state of widgets on main form (Connect, Disconnect and AVR Controls)
        void SetAVRControlsEnabled(bool isEnabled);
        void SetConnectItemEnabled(bool isEnabled);
        void SetDisconnectItemEnabled(bool isEnabled);
    };
}
