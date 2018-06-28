#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include "avrmessage.h"
#include "avrsystem.h"

namespace AVR
{
    //This is AVR Server class. It implements connection between controlling interface and AVR System.
    //AVR Server could have only one client, other client's connections would be rejected.
    //Server uses TCP connection and could be hosted even through the internet.
    class Server : public QObject
    {
        Q_OBJECT

    private:
        QTcpServer* m_ptcpServer;   //The server instance.
        quint16 m_nNextBlockSize;   //Data block size (needed for internal server work)
        QTcpSocket* m_theOnlyClient;    //Current only client. This is socket linked to connected client if it exists.
        bool m_bHasClient;  //State of server. Does it have client or not.

    private:
        void sendToClient(QTcpSocket* pSocket, const QString& str); //Sends data to connected client
        AVR::Message FormMessage(const QString& str);  //Forms AVR::Message instance from incoming message of client.

    public:
        //Server's ctor, accepts host and port for listening.
        Server(const QHostAddress& host, int nPort, QObject* pwgt =0);
        ~Server();

        enum Exeption   //Server's exeptions
        {
            ListenFailed
        };

    public slots:
        void slotNewConnection();   //Slot of new incoming connection. Triggers when someone connects.
        void OnClientDisconnected();        //Triggers when client has been disconnected.
        void slotReadClient();              //Retrieving data from client

        void AVRWorkIsComplete();           //Triggers when AVR finished moving
        void OnAVRError(AVRSystem::Exeption code);          //Triggers when AVR error occurred
        void SendPosition(int pos);         //Sends current AVR position to client
        void OnMessageReceived(Message::Type type, int ReceivedSteps);  //Triggers when AVR system recieved message.
        void OnClientInit(int currentPos, int maxPos);  //Triggers when AVR system says to
                                                        //client it's current position and max position.
                                                        //Position on client init is ALWAYS true.
    signals:
        void AVRMessage(AVR::Message msg);  //Sends formed AVR::Message from client to AVR system
        void ChangeConnectionLabel(bool IsConnected);   //Says to UI form to change connection label's state text.
        void AskForClientInit();       //Says to AVR system to emit signal for server with it's current
                                       //position and maximum position. This position is ALWAYS true.
                                       //Server will send this data to client for it's initialization.
    };
}
