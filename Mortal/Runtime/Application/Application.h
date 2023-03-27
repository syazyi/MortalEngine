#pragma once
#include "Core/Core.h"
#include "Window/WindowsWindow.h"
#include <memory>
namespace mortal{

    class MORTAL_API Application{
    public:
        Application(Window* window);

        virtual void Run();
        void OnEvent(Event& e);
        bool ShouldClose{false};
    private:
        std::unique_ptr<Window> m_window;
    };

}//namespace mortal

