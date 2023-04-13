#include "rendering_system.h"
#include "Rendering/rendering_device.h"
#include "Window/WindowsWindow.h"
namespace mortal
{

    RenderingSystem::RenderingSystem() : Layer("Render Layer")
    {
        SetUpVulkan();
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
    }

    void RenderingSystem::ClearUpVulkan()
    {
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

            SetDebugCallBack();
        }
    }

    void RenderingSystem::DestroyInstance()
    {
        GetAndExecuteFunction<PFN_vkDestroyDebugUtilsMessengerEXT>("vkDestroyDebugUtilsMessengerEXT", callback, nullptr);
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

} // namespace mortal
