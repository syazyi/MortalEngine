#pragma once 
#include "Events/Event.h"
#include <iostream>
#define EVENT_SET_FUNCTION(name) \
    EventType GetType() const override{ return m_type; } \
    const char* GetName() const{ return m_name; } \
    const char* m_name = #name;
namespace mortal
{
    class WindowResizeEvent : public Event{
    public:
        WindowResizeEvent(const int width, const int height) : m_width(width), m_height(height){}
        EVENT_SET_FUNCTION(WindowResizeEvent)
        virtual std::string ToString() const {
            std::stringstream ss;
            ss << m_name << ": " << m_width << ", " << m_height << "\n";
            return ss.str();
        }
        int m_width;
        int m_height;
    private:
        static constexpr EventType m_type = EventType::EWindowResize;
    };

    class WindosCloseEvent : public Event {
    public:
        WindosCloseEvent() = default;
        EVENT_SET_FUNCTION(WindosCloseEvent)
        virtual std::string ToString() const {
            std::stringstream ss;
            ss << m_name << "\n";
            return ss.str();
        }
    private:
        static constexpr EventType m_type = EventType::EWindowColse;
    };

    class KeyEvent : public Event {
    public:
        int GetKey(){
            return m_key;
        }
    protected:
        KeyEvent(int Key) : m_key(Key) {}
        int m_key;
    };

    class KeyPressEvent : public KeyEvent {
    public:
        explicit KeyPressEvent(int Key) : KeyEvent(Key) {}
        EVENT_SET_FUNCTION(KeyPressEvent)
        virtual std::string ToString() const {
            std::stringstream ss;
            ss << m_name << "\n";
            return ss.str();
        }
    private:
        
        static constexpr EventType m_type = EventType::EKeyPress;
    };

    class KeyReleaseEvent : public KeyEvent {
    public:
        explicit KeyReleaseEvent(int Key) : KeyEvent(Key) {}
        EVENT_SET_FUNCTION(KeyReleaseEvent)
            virtual std::string ToString() const {
            std::stringstream ss;
            ss << m_name << "\n";
            return ss.str();
        }
    private:
        static constexpr EventType m_type = EventType::EKeyRelease;
    };

    class MouseMovedEvent : public Event{
    public:
        MouseMovedEvent(double xPos, double yPos) : x_pos(xPos), y_pos(yPos){}
        EVENT_SET_FUNCTION(MouseMovedEvent)
        virtual std::string ToString() const {
            std::stringstream ss;
            ss << m_name << ": " << x_pos << ", " << y_pos << "\n";
            return ss.str();
        }
        double GetXPos() {
            return x_pos;
        }
        double GetYPos() {
            return y_pos;
        }
    private:
        double x_pos;
        double y_pos;
        static constexpr EventType m_type = EventType::EMouseMoved;
    };

} // namespace mortal
