#pragma once 

namespace mortal
{
    enum class EventType{
        EWindowResize
    } ;


    class Event{
    public: 
        virtual EventType GetType() const = 0;
        virtual ~Event(){}
    };

    

}