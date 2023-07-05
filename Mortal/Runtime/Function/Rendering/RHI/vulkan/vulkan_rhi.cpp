#include "vulkan_rhi.h"
namespace mortal
{
    VulkanRHI::~VulkanRHI()
    {
        //Clearup all infomation
        auto& device = m_Context.device.GetDevice();
        for (uint32_t i = 0; i < MaxFrameInFlight; i++) {
            device.destroyFence(m_Synchronizations.m_FrameFences[i]);
            device.destroySemaphore(m_Synchronizations.m_PresentSemaphores[i]);
            device.destroySemaphore(m_Synchronizations.m_GetImageSemaphores[i]);
        }

        m_Context.command.ClearUp();
        m_Context.swapchain.ClearUp();
        m_Context.device.ClearUp();
        m_Context.window.ClearUp(m_Instance);
        DestroyInstance();
    }

    void VulkanRHI::Init()
    {
        CreateInstance();
        CreateWindowSurface();
        CreateDevice();
        CreateSwapchain();
        CreateCommandPool();
        CreateSynchronization();
        CreateGlobalDescription();
    }

    //private
    void VulkanRHI::CreateInstance()
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

    bool VulkanRHI::CheckValidtionLayer()
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

    std::vector<const char*> VulkanRHI::GetRequireExtensions()
    {
        uint32_t extensionsCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionsCount);
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + extensionsCount);
        if constexpr (EnableValidtion) {
            extensions.push_back("VK_EXT_debug_utils");
        }
        return extensions;
    }

    void VulkanRHI::SetDebugCallBack()
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
            GetAndExecuteFunction<PFN_vkCreateDebugUtilsMessengerEXT>("vkCreateDebugUtilsMessengerEXT", m_Instance, &debugUtilsCreateInfo,
                nullptr,
                &callback);
        }
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
        std::cerr << "validation layer: \n" << pCallbackData->pMessage << "\n\n";
        return VK_FALSE;
    }

    void VulkanRHI::CreateWindowSurface()
    {
        m_Context.window.SetWindow(m_Instance);
    }

    void VulkanRHI::CreateDevice()
    {
        m_Context.device.SetDevice(m_Instance, m_Context.window.GetSurface());
    }

    void VulkanRHI::CreateSwapchain()
    {
        m_Context.swapchain.Create(m_Context.device, m_Context.window);
    }

    void VulkanRHI::CreateCommandPool()
    {
        m_Context.command.SetCommandPool(m_Context.device);
    }

    void VulkanRHI::CreateSynchronization()
    {
        //create semaphore and fence
        auto& device = m_Context.device.GetDevice();
        vk::SemaphoreCreateInfo createInfo{};
        vk::FenceCreateInfo fCreateInfo(vk::FenceCreateFlagBits::eSignaled);
        for (uint32_t i = 0; i < MaxFrameInFlight; i++) {
            m_Synchronizations.m_GetImageSemaphores[i] = device.createSemaphore(createInfo);
            m_Synchronizations.m_PresentSemaphores[i] = device.createSemaphore(createInfo);
            m_Synchronizations.m_FrameFences[i] = device.createFence(fCreateInfo);
        }
        m_Context.SemphoreInfo = &m_Synchronizations;
    }

    void VulkanRHI::CreateGlobalDescription()
    {
        //create description
        auto& device = m_Context.device.GetDevice();

        std::vector<vk::DescriptorPoolSize> poolsizes;
        poolsizes.reserve(3);
        poolsizes.emplace_back(vk::DescriptorPoolSize{ vk::DescriptorType::eUniformBuffer, 128 });
        poolsizes.emplace_back(vk::DescriptorPoolSize{ vk::DescriptorType::eUniformBufferDynamic, 128 });
        poolsizes.emplace_back(vk::DescriptorPoolSize{ vk::DescriptorType::eCombinedImageSampler, 128 });
        uint32_t maxsets = 0;
        for (auto& poolsize : poolsizes) {
            maxsets += poolsize.descriptorCount;
        }
        m_Context.DescriptorPool = device.createDescriptorPool(vk::DescriptorPoolCreateInfo{ {}, maxsets, poolsizes });
    }

    void VulkanRHI::DestroyInstance()
    {
        if constexpr (EnableValidtion) {
            GetAndExecuteFunction<PFN_vkDestroyDebugUtilsMessengerEXT>("vkDestroyDebugUtilsMessengerEXT", m_Instance,callback, nullptr);
        }
        m_Instance.destroy();
    }

    //The above information has been discarded¡¡later
    namespace renderAPI {
        //instance
        struct VkInstanceUtil {
            VkInstanceUtil() {
                if constexpr (EnableValidtion) {
                    instance_layers.emplace_back("VK_LAYER_KHRONOS_validation");
                    instance_extensions.emplace_back("VK_EXT_debug_utils");

                    debugUtilsCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
                    debugUtilsCreateInfo.messageSeverity = VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                        VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                        VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
                    debugUtilsCreateInfo.messageType = VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                        VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                        VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
                    debugUtilsCreateInfo.pfnUserCallback = &debugCallback;
                }
            }
            std::vector<const char*> instance_layers;
            std::vector<const char*> instance_extensions;
            VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo;
        };

        RenderInstance* CreateInstance_Vulkan(const CreateInstanceDescriptor* desc)
        {
            VkInstanceUtil utilInfo;
            auto& instanceLayers = utilInfo.instance_layers;
            if constexpr (EnableValidtion) {
                std::vector<vk::ExtensionProperties> layerNames = vk::enumerateInstanceExtensionProperties();
                bool result = false;
                for (auto& sLayerName : instanceLayers) {
                    for (auto& layerName : layerNames) {
                        if (strcmp(layerName.extensionName, sLayerName) == 0) {
                            result |= true;
                        }
                    }
                }
            }
            //create
            vk::ApplicationInfo appInfo("Mortal", 1, "Mortal Engine", 1, VK_API_VERSION_1_3);

            //only glfw , need improve
            uint32_t extensionsCount = 0;
            const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionsCount);
            auto& instanceExtensions = utilInfo.instance_extensions;
            std::vector<const char*> extensions(glfwExtensions, glfwExtensions + extensionsCount);
            instanceExtensions.insert(instanceExtensions.end(), extensions.begin(), extensions.end());

            vk::InstanceCreateInfo createInfo({}, &appInfo, instanceLayers, instanceExtensions);

            auto* riv = new RenderInstance_Vulkan;
            riv->instance = vk::createInstance(createInfo);

            if constexpr (EnableValidtion) {
                GetAndExecuteFunction<PFN_vkCreateDebugUtilsMessengerEXT>("vkCreateDebugUtilsMessengerEXT", riv->instance, &utilInfo.debugUtilsCreateInfo,
                    nullptr,
                    &riv->callback_vulkan);
            }
            return reinterpret_cast<RenderInstance*>(riv);
        }
        void FreeInstance_Vulkan(const RenderInstance* ri)
        {
            auto riv = reinterpret_cast<const RenderInstance_Vulkan*>(ri);
            if constexpr (EnableValidtion) {
                GetAndExecuteFunction<PFN_vkDestroyDebugUtilsMessengerEXT>("vkDestroyDebugUtilsMessengerEXT", riv->instance, riv->callback_vulkan, nullptr);
            }
            riv->instance.destroy();
        }

        //End of Instance
        // 
        // Physical(adapter)
        void EnumPhysicalDevice_Vulkan(RenderInstance* ri, const RenderPhysicalDevice* rpd)
        {

        }

        //End of Physical(adapter)

    }//namespcae renderAPI

} // namespace mortal

