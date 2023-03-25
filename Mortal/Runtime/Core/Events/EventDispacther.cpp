#include "Events/EventDispatcher.h"

namespace mortal
{
    void EventDispatcher::Subscribe(EventType type, CallbackFunc func){
        m_dispatchMap[type] = func;
    }

    void EventDispatcher::Dispatch(Event& event){
        auto it = m_dispatchMap.find(event.GetType());
        if(it != m_dispatchMap.end()){
            it->second(event);
        }
    }

} // namespace mortal
