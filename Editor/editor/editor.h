#pragma once
#include <memory>
#include "Core/Core.h"
#include "Window/WindowsWindow.h"
#include "Layer/LayerStack.h"
namespace mortal{

    class Editor final{
    public:
        Editor(Window* window);
        ~Editor(){}
        void Run();
        void OnEvent(Event& e);
        bool ShouldClose{false};

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* layer);
    private:
        LayerStack m_stack;
        std::unique_ptr<Window> m_window;
    };

    //class ExamLayer : public Layer {
    //public:
    //    ExamLayer();
    //    void OnUpdate() override;
    //    void OnEvent(Event& e) override;
    //};
}//namespace mortal

