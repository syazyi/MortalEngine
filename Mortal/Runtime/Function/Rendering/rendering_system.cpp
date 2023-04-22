#include "rendering_system.h"
#include "Rendering/rendering_device.h"
#include "Window/WindowsWindow.h"

#include "rendering_part_base.h"
#include "Rendering/Pass/triangle.h"
namespace mortal
{
    void RenderingSystem::OnUpdate()
    {
        auto& device = m_Info.device.GetDevice();
        auto& currentFrame = m_Info.CurrentFrame;
        auto result_waitFence = device.waitForFences(m_Info.m_FrameFences[currentFrame], VK_TRUE, UINT64_MAX);

        auto& swapchain = m_Info.swapchain.GetSwapChain();
        auto result_nextImageIndex = device.acquireNextImageKHR(swapchain, UINT64_MAX, m_Info.m_GetImageSemaphores[currentFrame]);
        m_Info.nextImageIndex = result_nextImageIndex.value;

        device.resetFences(m_Info.m_FrameFences[currentFrame]);

        auto& drawCmd = m_Info.command.GetCommandBuffers()[currentFrame];
        drawCmd.reset();


        for (auto& pass : m_RenderParts) {
            pass->Draw();
        }

        auto& drawQueue = m_Info.device.GetRenderingQueue().PresentQueue.value();

        std::array<vk::PipelineStageFlags, 1> pipelineStages{ vk::PipelineStageFlagBits::eColorAttachmentOutput };
        vk::SubmitInfo subInfo(m_Info.m_GetImageSemaphores[currentFrame], pipelineStages, drawCmd, m_Info.m_PresentSemaphores[currentFrame]);
        drawQueue.submit(subInfo, m_Info.m_FrameFences[currentFrame]);

        vk::PresentInfoKHR presentInfo(m_Info.m_PresentSemaphores[currentFrame], swapchain, m_Info.nextImageIndex);
        auto result_present = drawQueue.presentKHR(presentInfo);

        currentFrame = (currentFrame + 1) % MaxFrameInFlight;

    }

    void RenderingSystem::OnEvent(Event& e)
    {

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
                m_Info.m_GetImageSemaphores[i] = device.createSemaphore(createInfo);
                m_Info.m_PresentSemaphores[i] = device.createSemaphore(createInfo);
                m_Info.m_FrameFences[i] = device.createFence(fCreateInfo);
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
            device.destroyFence(m_Info.m_FrameFences[i]);
            device.destroySemaphore(m_Info.m_PresentSemaphores[i]);
            device.destroySemaphore(m_Info.m_GetImageSemaphores[i]);
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
            vk::DebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo({},
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning,
                vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
                &debugCallback);

            GetAndExecuteFunction<PFN_vkCreateDebugUtilsMessengerEXT>("vkCreateDebugUtilsMessengerEXT", &(VkDebugUtilsMessengerCreateInfoEXT)debugUtilsCreateInfo,
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
        AddRenderPart(new TrianglePart(m_Info));
    }

} // namespace mortal
