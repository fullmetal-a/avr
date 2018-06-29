#include "avrmessage.h"

namespace AVR
{
    Message::Message()
    {
        m_Type = Message::Type::Unknown;
        m_stepCount = 0;
    }

    Message::Message(Message::Type type, int steps)
    {
        m_Type = type;
        m_stepCount = steps;
    }

    Message::Message(const Message &copy)   //Copy ctor
    {
        m_Type = copy.m_Type;
        m_stepCount = copy.m_stepCount;
    }

    Message::~Message() //No data to destroy
    {
    }

    Message& Message::operator=(const Message& msg)
    {
        m_Type = msg.m_Type;
        m_stepCount = msg.m_stepCount;
        return *this;
    }

    Message::Type Message::GetMessageType() const
    {
        if(m_Type > Message::Type::Unknown &&
           m_Type < Message::Type::TYPE_MAX)
            return m_Type;  //Other types are not supported.
        else
            return Message::Type::Unknown;  //Unknown message received
    }

    int Message::GetSteps() const
    {
        return m_stepCount;
    }
}
