#pragma once

#include <QObject>
#include <QLCDNumber>
#include "avrmessage.h"

namespace AVR
{
    //Class of AVR System. This is main unit of AVR emulator.
    //It contains all AVR logic, works in separate thread and communicates with server and UI.
    //AVRSystem class accepts client messages from servers and replies to them.
    class AVRSystem : public QObject
    {
        Q_OBJECT

    public:
        enum class State  //State of AVR system. It can idle or be in movement.
        {
            Idle,
            Moving
        };

        enum class Error   //Exepting situation codes for AVR System
        {
            UnknownMessage,
            ValueIsLowerThanZero,
            TooHighValue,
            AlreadyMoving
        };

    private:
        AVRSystem::State m_State;   //Current state of AVR system
        int m_iCurrentPosition;     //Current position
        int m_iGoalPosition;        //Goal position (future current position, becomes it when AVR finished moving)
        int m_iChanceToLie;         //Chance to lie (must be between 1 and 100)
        int m_iMaxPos;              //Maximum possible position

        //Disallow evil constructors + default
        AVRSystem(void) = delete;
        AVRSystem(const AVRSystem&) = delete;
        AVRSystem& operator=(const AVRSystem&) = delete;
        AVRSystem(AVRSystem&&) = delete;
        AVRSystem& operator=(AVRSystem&&) = delete;

        //Internal private methods
        void MoveToZero();          //Begins moving AVR position to 0
        void MoveToPos(int pos);    //Begins moving AVR position to specific position
        int GetCurrentPos() const;  //Returns current AVR position.
                                    //With chance of m_iChanceToLie it can say wrong position.
                                    //On zero position it always says true position.

    public:
        //Constructor initiates AVRSystem with chance to lie and maximum position.
        AVRSystem(int ChanceToLie, int MaxPos, QObject* parent = 0);
        ~AVRSystem();
        

    public slots:
        void ParseMsg(Message msg); //Parses incoming client's message from server.
        void OnClientInitRequest(); //Triggers when server asks for client init when it was connected.
                                    //Always sends true

    signals:
        void WorkIsComplete();        //Says to server when moving was complete.
        void SendPosition(int pos);   //Says to server current position (calls GetCurrentPos() method)
        void ErrorOccurred(AVRSystem::Error code); //Says to server when error occured in AVR System.
        void UpdateDisplay(int pos);  //Asks UI to update position value
        void MessageReceived(Message::Type type, int ReceivedSteps = 0);    //Reports server that messsage from client was received (What message and how much steps).
        void ClientInit(int currentPos, int maxPos);    //Sends server information for initializing client.
    };

}

