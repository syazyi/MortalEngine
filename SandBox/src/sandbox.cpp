#include "sandbox.h"
#include "Rendering/rendering_system.h"
namespace mortal {

	SandBox::SandBox(Window* window) : Application(window)
	{
		PushLayer(new ExamLayer());
		PushLayer(RenderingSystem::GetInstance());
	}

	ExamLayer::ExamLayer() : Layer("examLayer")
	{
		
	}

	void ExamLayer::OnUpdate()
	{
		MORTAL_LOG_INFO("ExamLayer::Update");
	}

	void ExamLayer::OnEvent(Event& e)
	{
		MORTAL_LOG_TRACE("{0}", e)
	}


}//namespace mortal