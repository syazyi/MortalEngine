#include "rendering_camera.h"
#include "glm/vec3.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "rendering_system.h"

namespace mortal
{
    void Camera::SetFront(glm::vec3 front)
    {
        m_Front = front;
    }

    void Camera::SetPos(glm::vec3 pos)
    {
        m_Pos = pos;
    }

    glm::mat4 Camera::GetView()
    {
        return glm::lookAt(m_Pos, m_Pos + m_Front, m_Up);
    }

    void Camera::SetFrontWithMousePos(float xPos, float yPos)
    {
        
        if (firstMouse)
        {
            lastX = xPos;
            lastY = yPos;
            firstMouse = false;
        }

        float xoffset = xPos - lastX;
        float yoffset = lastY - yPos;
        lastX = xPos;
        lastY = yPos;

        float sensitivity = 0.05;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;
        pitch += yoffset;

        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        glm::vec3 front;
        //front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        //front.y = -sin(glm::radians(pitch));
        //front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = -sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.z = sin(glm::radians(pitch));
        m_Front = glm::normalize(front);
    }


} // namespace mortal
