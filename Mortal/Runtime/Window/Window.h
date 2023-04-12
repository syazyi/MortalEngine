#pragma once
#include <cstdint>
#include <functional>
#include "Core.h"
#include "Events/Event.h"
namespace mortal
{
    using EventCallback = std::function<void(Event&)>;
    class MORTAL_API Window{
    public:
        
        virtual ~Window(){}

        virtual void SetCallback(EventCallback func) = 0;

        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;

        virtual void Update() = 0;
        virtual bool CheckShouldBeClose() = 0;
        //virtual void SetVSync(bool enable) = 0;
        //virtual bool isVSync() = 0;
    public:
        struct WindowCreateInfo
        {
            const char* pTitle;
            uint32_t width;
            uint32_t height;

            EventCallback callfunc{nullptr};
        };

        static Window* Create(const WindowCreateInfo& info);

    }; 
    
} // namespace mortal
