#pragma once

namespace AVR
{
    class Message   //Class of incoming message instance.
                    //All incoming messages from client to AVR system must be translated to AVR::Message.
    {
    public:
        enum class Type    //All possible messages
        {
            Unknown,
            MoveForNSteps,
            MoveToZero,
            GetPosition,
            TYPE_MAX
        };

    private:
        Message::Type m_Type; //Current message
        int m_stepCount; //Additional field for step value in case of MoveForNSteps message type

    public:
        //Default, custom and copy ctors
        Message();
        Message(Message::Type type, int steps = 0);
        Message(const Message &copy);

        ~Message();

        Message& operator=(const Message& msg);

        Message::Type GetMessageType() const;   //Returns type of message
        int GetSteps() const;   //Return count of steps of this message.
    };
}
