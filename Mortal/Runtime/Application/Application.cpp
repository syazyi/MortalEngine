#include "Application.h"

#include "Events/WindowResizeEvent.h"
#include "Events/EventDispatcher.h"
namespace mortal
{
    void OnWindowResizeEvent(Event& event){
        auto windowResizeEvent = dynamic_cast<WindowResizeEvent&>(event);
        MORTAL_LOG_INFO("%d, %d", windowResizeEvent.m_width, windowResizeEvent.m_height);
        //printf("%d, %d", windowResizeEvent.m_width, windowResizeEvent.m_height);
    }
void Application::Run(){
    EventDispatcher dispatch;
    MORTAL_LOG_INFO("Running...")
    
    WindowResizeEvent wre;
    wre.m_width = 100;
    wre.m_height = 50;

    dispatch.Subscribe(EventType::EWindowResize, OnWindowResizeEvent);

    dispatch.Dispatch(wre);
    MORTAL_LOG_INFO("End!")
}



} // namespace mortal
