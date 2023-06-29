#include "rendering_system.h"
#include "Rendering/rendering_device.h"
#include "Window/WindowsWindow.h"

#include "rendering_part_base.h"
#include "Rendering/Part/Triangle/triangle.h"
#include "Rendering/Part/BlingPhong/blingphong.h"
#include "Rendering/Part/ShadowMapping/shadow_mapping.h"
#include "Rendering/Part/Particles/particles.h"
#include "Core/Events/WindowEvent.h"
namespace mortal
{
    void RenderingSystem::SetUpRHI()
    {
        m_RHIContext->Init();
    }

    void RenderingSystem::OnUpdate()
    {
        CameraMove();

        for (auto& pass : m_RenderParts) {
            pass->Draw();
        }

        auto rhi = std::static_pointer_cast<VulkanRHI>(m_RHIContext);
        rhi->m_Context.CurrentFrame = (rhi->m_Context.CurrentFrame + 1) % MaxFrameInFlight;

    }

    void RenderingSystem::OnEvent(Event& e)
    {
        auto rhi = std::static_pointer_cast<VulkanRHI>(m_RHIContext);
        if (e.GetType() == EventType::EMouseMoved && glfwGetMouseButton(rhi->m_Context.window.GetWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            auto& event = reinterpret_cast<MouseMovedEvent&>(e);
            rhi->m_Context.m_Camera.SetFrontWithMousePos(event.GetXPos(), event.GetYPos());
            //MORTAL_LOG_INFO("{0}", e);
        }
        if (glfwGetMouseButton(rhi->m_Context.window.GetWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
            rhi->m_Context.m_Camera.firstMouse = true;
        }

    }

    RenderingSystem::RenderingSystem() : Layer("Render Layer")
    {
        //set vulakn and directx
        if constexpr (1) {
            m_RHIContext = std::make_shared<VulkanRHI>();
        }

        SetUpRHI();
        AddRenderPasses();
    }

    RenderingSystem::~RenderingSystem()
    {
        for (auto& renderPass : m_RenderParts) {
            renderPass.reset();
        }
    }


    void RenderingSystem::AddRenderPasses()
    {
        auto rhi = std::static_pointer_cast<VulkanRHI>(m_RHIContext);
        //AddRenderPart(new TrianglePart(rhi->m_Context));
        //AddRenderPart(new UI(m_Info));
        AddRenderPart(new BlingPhong(rhi->m_Context));
        //AddRenderPart(new ShadowPart(m_Info));
        //AddRenderPart(new ParticlesPart(m_Info));
    }

    void RenderingSystem::CameraMove()
    {
        auto rhi = std::static_pointer_cast<VulkanRHI>(m_RHIContext);
        auto* window = rhi->m_Context.window.GetWindow();
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            rhi->m_Context.m_Camera.m_Pos += rhi->m_Context.m_Camera.m_Speed * rhi->m_Context.m_Camera.m_Front;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            rhi->m_Context.m_Camera.m_Pos -= rhi->m_Context.m_Camera.m_Speed * rhi->m_Context.m_Camera.m_Front;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            rhi->m_Context.m_Camera.m_Pos -= glm::normalize(glm::cross(rhi->m_Context.m_Camera.m_Front, rhi->m_Context.m_Camera.m_Up)) * rhi->m_Context.m_Camera.m_Speed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            rhi->m_Context.m_Camera.m_Pos += glm::normalize(glm::cross(rhi->m_Context.m_Camera.m_Front, rhi->m_Context.m_Camera.m_Up)) * rhi->m_Context.m_Camera.m_Speed;
    }

} // namespace mortal
