#pragma once
#include <memory>
#include "Core/Core.h"
#include "Window/WindowsWindow.h"
#include "Layer/LayerStack.h"
namespace mortal{

    class MORTAL_API Application{
    public:
        Application(Window* window);
        virtual ~Application(){}
        virtual void Run();
        void OnEvent(Event& e);
        bool ShouldClose{false};

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* layer);
    private:
        LayerStack m_stack;
        std::unique_ptr<Window> m_window;
    };

}//namespace mortal

