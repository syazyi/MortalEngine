#pragma once
#include "rendering.h"

namespace mortal
{
	struct Camera
	{
		void SetFront(glm::vec3 front);
		void SetPos(glm::vec3 pos);
		glm::mat4 GetView();
		void SetFrontWithMousePos(float xPos, float yPos);
		glm::vec3 m_Front{ -1.0f, 0.0f, 0.0f };
		glm::vec3 m_Pos{ 5.0f, 0.0f, 0.0f };
		glm::vec3 m_Up{ 0.0f, 0.0f, 1.0f };
		float m_Speed{0.005f};

		bool firstMouse{true};
		float lastX{800.f};
		float lastY{450.f};
		float yaw{ 0.0f };
		float pitch{ 0.0f};
	};
} // namespace mortal
