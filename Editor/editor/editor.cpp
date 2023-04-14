#include "Editor.h"

#include <functional>
#include "Events/WindowEvent.h"
#include "Events/EventDispatcher.h"
#include "Rendering/rendering_system.h"
namespace mortal
{

    Editor::Editor(Window* window) : m_window(window)
    {
        //m_window->SetCallback([](Event& event){});

        m_window->SetCallback(std::bind(&Editor::OnEvent, this, std::placeholders::_1));
        //PushLayer(new ExamLayer());

		PushLayer(RenderingSystem::GetInstance());
    }
    void Editor::Run(){
        auto& ep = EventDispatcher::GetInstance();
        ep.Subscribe(EventType::EWindowColse, [this](Event& e) {
            this->ShouldClose = true;
            });
        MORTAL_LOG_INFO("Running...")

        while (!ShouldClose) {
            //dispatch.Dispatch(wre);

            for (auto layer : m_stack) {
                layer->OnUpdate();
            }
            m_window->Update();
        }
        MORTAL_LOG_INFO("End!")
    }

    void Editor::OnEvent(Event& e)
    {
        EventDispatcher::GetInstance().Dispatch(e);
        
        MORTAL_LOG_INFO("{0}", e);

        for (auto layer : m_stack) {
            layer->OnEvent(e);
        }
    }

    void Editor::PushLayer(Layer* layer)
    {
        m_stack.PushLayer(layer);
    }

    void Editor::PushOverlay(Layer* layer)
    {
        m_stack.PushOverlay(layer);
    }

 //   	ExamLayer::ExamLayer() : Layer("examLayer")
	//{
	//	
	//}

	//void ExamLayer::OnUpdate()
	//{
	//	//MORTAL_LOG_INFO("ExamLayer::Update");
	//}

	//void ExamLayer::OnEvent(Event& e)
	//{
	//	//MORTAL_LOG_TRACE("{0}", e)
	//}

} // namespace mortal
