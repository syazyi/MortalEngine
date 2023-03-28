#include "sandbox.h"

namespace mortal {

	SandBox::SandBox(Window* window) : Application(window)
	{
		PushLayer(new ExamLayer());
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