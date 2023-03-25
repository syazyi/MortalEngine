#pragma once 
#include "Events/Event.h"
namespace mortal
{
    class WindowResizeEvent : public Event{
    public:
        EventType GetType() const override{
            return m_type;
        }
        int m_width;
        int m_height;
    private:
        static constexpr EventType m_type = EventType::EWindowResize;
    };


} // namespace mortal
