#include "Application.h"

#include <functional>
#include "Events/WindowEvent.h"
#include "Events/EventDispatcher.h"
namespace mortal
{

    Application::Application(Window* window) : m_window(window)
    {
        //m_window->SetCallback([](Event& event){});

        m_window->SetCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));
    }
    void Application::Run(){
    auto& ep = EventDispatcher::GetInstance();
    ep.Subscribe(EventType::EWindowColse, [this](Event& e) {
        this->ShouldClose = true;
        });
    MORTAL_LOG_INFO("Running...")

    while (!ShouldClose) {
        //dispatch.Dispatch(wre);
        m_window->Update();
    }
    MORTAL_LOG_INFO("End!")
}

    void Application::OnEvent(Event& e)
    {
        EventDispatcher::GetInstance().Dispatch(e);
        
        MORTAL_LOG_INFO("{0}", e);
    }


} // namespace mortal
