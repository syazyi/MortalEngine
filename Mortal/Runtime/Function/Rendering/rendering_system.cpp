#include "rendering_system.h"
#include "Rendering/rendering_device.h"
#include "Window/WindowsWindow.h"

#include "rendering_part_base.h"
#include "Rendering/Part/Triangle/triangle.h"
#include "Rendering/Part/BlingPhong/blingphong.h"
#include "Rendering/Part/UI/ui.h"
#include "Rendering/Part/ShadowMapping/shadow_mapping.h"
#include "Core/Events/WindowEvent.h"
namespace mortal
{
    void RenderingSystem::OnUpdate()
    {
        CameraMove();

        auto& device = m_Info.device.GetDevice();
        auto& currentFrame = m_Info.CurrentFrame;
        auto result_waitFence = device.waitForFences(m_Synchronizations.m_FrameFences[currentFrame], VK_TRUE, UINT64_MAX);

        auto& swapchain = m_Info.swapchain.GetSwapChain();
        auto result_nextImageIndex = device.acquireNextImageKHR(swapchain, UINT64_MAX, m_Synchronizations.m_GetImageSemaphores[currentFrame]);
        m_Info.nextImageIndex = result_nextImageIndex.value;

        device.resetFences(m_Synchronizations.m_FrameFences[currentFrame]);

        auto& drawCmd = m_Info.command.GetCommandBuffers()[currentFrame];
        drawCmd.reset();


        for (auto& pass : m_RenderParts) {
            pass->Draw();
        }

        auto& drawQueue = m_Info.device.GetRenderingQueue().PresentQueue.value();

        std::array<vk::PipelineStageFlags, 1> pipelineStages{ vk::PipelineStageFlagBits::eColorAttachmentOutput };
        vk::SubmitInfo subInfo(m_Synchronizations.m_GetImageSemaphores[currentFrame], pipelineStages, drawCmd, m_Synchronizations.m_PresentSemaphores[currentFrame]);
        drawQueue.submit(subInfo, m_Synchronizations.m_FrameFences[currentFrame]);

        vk::PresentInfoKHR presentInfo(m_Synchronizations.m_PresentSemaphores[currentFrame], swapchain, m_Info.nextImageIndex);
        auto result_present = drawQueue.presentKHR(presentInfo);

        currentFrame = (currentFrame + 1) % MaxFrameInFlight;

    }

    void RenderingSystem::OnEvent(Event& e)
    {
        if (e.GetType() == EventType::EMouseMoved && glfwGetMouseButton(m_Info.window.GetWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            auto& event = reinterpret_cast<MouseMovedEvent&>(e);
            m_Info.m_Camera.SetFrontWithMousePos(event.GetXPos(), event.GetYPos());
            //MORTAL_LOG_INFO("{0}", e);
        }
        if (glfwGetMouseButton(m_Info.window.GetWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
            m_Info.m_Camera.firstMouse = true;
        }

    }

    RenderingSystem::RenderingSystem() : Layer("Render Layer")
    {
        SetUpVulkan();
        AddRenderPasses();
    }

    RenderingSystem::~RenderingSystem()
    {
        ClearUpVulkan();
    }

    void RenderingSystem::SetUpVulkan()
    {
        CreateInstance();
        m_Info.window.SetWindow(m_Instance);
        m_Info.device.SetDevice(m_Instance, m_Info.window.GetSurface());
        m_Info.swapchain.Create(m_Info.device, m_Info.window);
        m_Info.command.SetCommandPool(m_Info.device);

        //create semaphore and fence
        {
            auto& device = m_Info.device.GetDevice();
            vk::SemaphoreCreateInfo createInfo{};
            vk::FenceCreateInfo fCreateInfo(vk::FenceCreateFlagBits::eSignaled);
            for (uint32_t i = 0; i < MaxFrameInFlight; i++) {
                m_Synchronizations.m_GetImageSemaphores[i] = device.createSemaphore(createInfo);
                m_Synchronizations.m_PresentSemaphores[i] = device.createSemaphore(createInfo);
                m_Synchronizations.m_FrameFences[i] = device.createFence(fCreateInfo);
            }
        }
    }

    void RenderingSystem::ClearUpVulkan()
    {
        for (auto& renderPass : m_RenderParts) {
            renderPass.reset();
        }

        auto& device = m_Info.device.GetDevice();
        for (uint32_t i = 0; i < MaxFrameInFlight; i++) {
            device.destroyFence(m_Synchronizations.m_FrameFences[i]);
            device.destroySemaphore(m_Synchronizations.m_PresentSemaphores[i]);
            device.destroySemaphore(m_Synchronizations.m_GetImageSemaphores[i]);
        }


        m_Info.command.ClearUp();
        m_Info.swapchain.ClearUp();
        m_Info.device.ClearUp();
        m_Info.window.ClearUp(m_Instance);
        DestroyInstance();
    }

//Set instance
    void RenderingSystem::CreateInstance()
    {
        if constexpr (EnableValidtion) {
            s_LayerNames.push_back("VK_LAYER_KHRONOS_validation");
            CheckValidtionLayer();
        }
        //Set Vulkan Instance
        {
            vk::ApplicationInfo appInfo("Mortal", 1, "Mortal Engine", 1, VK_API_VERSION_1_3);

            auto extensions = GetRequireExtensions();
            vk::InstanceCreateInfo createInfo({}, &appInfo, s_LayerNames, extensions);

            m_Instance = vk::createInstance(createInfo);

            if constexpr (EnableValidtion) {
                SetDebugCallBack();
            }
        }
    }

    void RenderingSystem::DestroyInstance()
    {
        if constexpr (EnableValidtion) {
            GetAndExecuteFunction<PFN_vkDestroyDebugUtilsMessengerEXT>("vkDestroyDebugUtilsMessengerEXT", callback, nullptr);
        }
        m_Instance.destroy();
    }

    std::vector<const char*> RenderingSystem::GetRequireExtensions()
    {
        uint32_t extensionsCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionsCount);
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + extensionsCount);
        if constexpr (EnableValidtion) {
            extensions.push_back("VK_EXT_debug_utils");
        }
        return extensions;
    }

//set valiadion layer infomation
    bool RenderingSystem::CheckValidtionLayer()
    {
        std::vector<vk::ExtensionProperties> layerNames = vk::enumerateInstanceExtensionProperties();
        bool result = false;
        for (auto& sLayerName : s_LayerNames) {
            for (auto& layerName : layerNames) {
                if (strcmp(layerName.extensionName, sLayerName) == 0) {
                    result |= true;
                }
            }
            if (result == false) {
                return false;
            }
        }
        return true;
    }

    void RenderingSystem::SetDebugCallBack()
    {
        if constexpr (EnableValidtion) {
            VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo{};
            debugUtilsCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debugUtilsCreateInfo.messageSeverity = VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
                VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
            debugUtilsCreateInfo.messageType = VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
            debugUtilsCreateInfo.pfnUserCallback = &debugCallback;
            GetAndExecuteFunction<PFN_vkCreateDebugUtilsMessengerEXT>("vkCreateDebugUtilsMessengerEXT", &debugUtilsCreateInfo,
                nullptr,
                &callback);
        }
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL RenderingSystem::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
        std::cerr << "validation layer: \n" << pCallbackData->pMessage << "\n\n";
        return VK_FALSE;
    }

    void RenderingSystem::AddRenderPasses()
    {
        //AddRenderPart(new TrianglePart(m_Info));
        //AddRenderPart(new UI(m_Info));
        //AddRenderPart(new BlingPhong(m_Info));
        AddRenderPart(new ShadowPart(m_Info));
    }

    void RenderingSystem::CameraMove()
    {
        auto* window = m_Info.window.GetWindow();
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            m_Info.m_Camera.m_Pos += m_Info.m_Camera.m_Speed * m_Info.m_Camera.m_Front;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            m_Info.m_Camera.m_Pos -= m_Info.m_Camera.m_Speed * m_Info.m_Camera.m_Front;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            m_Info.m_Camera.m_Pos -= glm::normalize(glm::cross(m_Info.m_Camera.m_Front, m_Info.m_Camera.m_Up)) * m_Info.m_Camera.m_Speed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            m_Info.m_Camera.m_Pos += glm::normalize(glm::cross(m_Info.m_Camera.m_Front, m_Info.m_Camera.m_Up)) * m_Info.m_Camera.m_Speed;
    }

} // namespace mortal
