#pragma once 
#include <map>
#include "Events/Event.h"
namespace mortal
{
    class EventDispatcher{
    public:
        EventDispatcher(const EventDispatcher&) = delete;
        EventDispatcher& operator=(const EventDispatcher&) = delete;

        EventDispatcher(EventDispatcher&&) = delete;
        EventDispatcher operator=(EventDispatcher&&) = delete;

        static EventDispatcher& GetInstance() {
            static EventDispatcher dispatcher;
            return dispatcher;
        }
        using CallbackFunc = std::function<void(Event&)>;
        void Dispatch(Event& event);
        void Subscribe(EventType type, CallbackFunc func);
    private:
        EventDispatcher() = default;
        std::map<EventType, CallbackFunc> m_dispatchMap; 
    };

    
} // namespace mortal
