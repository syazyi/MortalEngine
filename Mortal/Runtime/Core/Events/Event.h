#pragma once 
#include "spdlog/fmt/ostr.h"
namespace mortal
{
    enum class EventType{
        EWindowResize, 
        EWindowColse,
        EKeyPress,
        EKeyRelease,
        EMouseMoved
    } ;

    class Event{
    public: 
        virtual EventType GetType() const = 0;
        virtual const char* GetName() const = 0;
        virtual std::string ToString() const { return GetName(); }
        virtual ~Event(){}

        friend std::ostream& operator<<(std::ostream& os, const Event& e)
        {
            return os << e.ToString();
        }
    };

    

}