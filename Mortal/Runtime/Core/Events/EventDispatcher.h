#pragma once 
#include <map>
#include "Events/Event.h"
namespace mortal
{
    class EventDispatcher{
    public:
        EventDispatcher() = default;

        using CallbackFunc = void(*)(Event&);
        void Dispatch(Event& event);
        void Subscribe(EventType type, CallbackFunc func);
    private:
        std::map<EventType, CallbackFunc> m_dispatchMap; 
    };

    
} // namespace mortal
