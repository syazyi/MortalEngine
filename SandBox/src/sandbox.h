#pragma once 
#include "Mortal.h"
namespace mortal
{
    class ExamLayer : public Layer {
    public:
        ExamLayer();
        void OnUpdate() override;
        void OnEvent(Event& e) override;
    };

    class SandBox : public Application{
    public:
        SandBox(Window* window);
    };

} // namespace mortal
