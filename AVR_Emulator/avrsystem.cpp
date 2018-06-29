#include "mainwindow.h"
#include "avrsystem.h"
#include <QMessageBox>

namespace AVR
{
    //Ctor of AVR System, takes chance to lie (when asking position) and maximum position value
    AVRSystem::AVRSystem(int ChanceToLie, int MaxPos, QObject *parent)
        : QObject(parent)
    {
        m_State = AVRSystem::State::Idle;
        m_iCurrentPosition = 0;
        m_iGoalPosition = 0;
        m_iChanceToLie = ChanceToLie;
        m_iMaxPos = MaxPos;
    }

    AVRSystem::~AVRSystem() //No data to destroy
    {
    }

    void AVRSystem::MoveToZero()    //Moves position to zero, uses MoveToPos()
    {
        MoveToPos(0);
    }

    //This method moves AVR position directly to pos value
    //Moving is not instant and could take a while.
    void AVRSystem::MoveToPos(int pos)
    {
        try //Let's try to move our system to pos value
        {
            if(pos < 0) //Throw and exeption if future position is lower than 0
                throw AVRSystem::Exeption::ValueIsLowerThanZero;
            else if(pos > m_iMaxPos)    //Throw and exeption if future position exceeds maximum position
                throw AVRSystem::Exeption::TooHighValue;

            //If future position equals current position
            if(pos == m_iCurrentPosition)
            {
                m_State = AVR::AVRSystem::State::Idle;  //Current state idle now
                emit WorkIsComplete();  //Saying server that work is complete
                return; //We are done
            }

            if(m_State == AVR::AVRSystem::State::Moving)    // Stop moving operation and throw an exeption if
                throw AVRSystem::Exeption::AlreadyMoving;   // system is alredy working.
                                                            // This situation is impossible in regular application
                                                            // life, but for some unusual cases this exeption will
                                                            // exist for avoiding unforeseen consequences.

            m_State = AVR::AVRSystem::State::Moving;  //Now we are moving
            m_iGoalPosition = pos;   //Set goal position to pos

            int waitTime = 1000;            // Initial pause in milliseconds between moving iterations.
            const int minumumWaitTime = 5;  // Wait time will be decreased on every iteration until
                                            // it will reach minumumWaitTime value.
            const float timeDecreaseFactor = 0.90f;   //This is factor of waitTime decrease per iteration.

            //This is move direction. Value equals 1 if system moving forward.
            //step value is actualy how much steps will be passed per iteration.
            int step = 1;
            if(m_iGoalPosition < m_iCurrentPosition)
                step = -1;  //Or if goal position is lower than current value will be -1. System moving backward.

            //We are begining from current position and moving until position isn't equal goal position
            for(int i = m_iCurrentPosition; i != (m_iGoalPosition + step); i += step)
            {
                m_iCurrentPosition = i;//Set current position to new iterated position

                //Decreasing wait time
                if(waitTime > minumumWaitTime)
                    waitTime *= timeDecreaseFactor;
                else if(waitTime < minumumWaitTime) //If time is lower than minimum value
                    waitTime = minumumWaitTime;     //than it equals minimum value.

                emit UpdateDisplay(i);  //Sending signal to UI for updating visible position value
                QThread::msleep(waitTime);   //Wait before next iteration
            }

            m_State = AVR::AVRSystem::State::Idle;  //Work is complete, now system is idling.
            emit WorkIsComplete();  //Sending signal to server for our client that work is complete
        }
        catch(AVRSystem::Exeption code)
        {
            emit ErrorOccurred(code);   //If any error occured - send information to server
        }
    }

    //This function is not a member of AVR::AVRSystem
    //It returns random integer value in range of two selected values
    int RandomBetween(int min, int max)
    {
        static bool rinit = false;
        if (!rinit)
        {
            qsrand(time(NULL));
            rinit = true;
        }
        return qrand() % ((max + 1) - min) + min;
    }

    //This method returns current position
    int AVRSystem::GetCurrentPos() const
    {
        int ResponsePos = m_iCurrentPosition;   //Initialy returned value will equal real position
        int toLieRoll = RandomBetween(1, 100);  //Now we getting random number between 1 and 100

        //If our dice roll chance value is lower or equals system's chance to lie
        //AND current real position is not 0
        //Than we are going to lie...
        if (toLieRoll <= m_iChanceToLie && m_iCurrentPosition > 0)
        {
            //Adding to response value some random number between -75 and 75
            ResponsePos += RandomBetween(-75, 75);
            if(ResponsePos > m_iMaxPos)     //If response position exceeds maximum position...
                ResponsePos = m_iMaxPos;    //Than it will be equal it.

            else if (ResponsePos < 0)       //If response position is lower than 0...
                ResponsePos = 0;            //Just set it to 0.
        }
        return ResponsePos; //Returning our response position
    }

    //This slot is parsing client's messages from server
    void AVRSystem::ParseMsg(Message msg)
    {
        try //Trying to parse it
        {
            Message::Type type = msg.GetMessageType();

            switch(type)
            {
                case Message::Type::MoveForNSteps:  //If asking for move for some steps
                    //Asking for server to say client that AVR system recieved his message
                    emit MessageReceived(type, msg.GetSteps());
                    MoveToPos(m_iCurrentPosition + msg.GetSteps()); //Moving to (Current position + Number of steps)
                    break;

                case Message::Type::MoveToZero:
                    //Asking for server to say client that AVR system recieved his message
                    emit MessageReceived(type);
                    MoveToZero();   //Moving to zero
                    break;

                case Message::Type::GetPosition:
                    //Asking for server to say client that AVR system recieved his message
                    emit MessageReceived(type);
                    emit SendPosition(GetCurrentPos()); //Returning to server position returned by GetCurrentPos()
                    break;

                case Message::Type::Unknown:
                default:
                    //Throw an exeption if message code is unknown
                    throw AVRSystem::Exeption::UnknownMessage;
            }
        }
        catch (AVRSystem::Exeption code)
        {
            emit ErrorOccurred(code);   //If any error occured - send information to server
        }
    }

    //This slot triggered when server asks initial data for new client
    void AVRSystem::OnClientInitRequest()
    {
        //Send to server current position and maximum position for new client
        //Sending new position directly from m_iCurrentPosition (not by GetCurrentPos() method)
        //So sent position will be always true.
        emit ClientInit(m_iCurrentPosition, m_iMaxPos);
    }

}
