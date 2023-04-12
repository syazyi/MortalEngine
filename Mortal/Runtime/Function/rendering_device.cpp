#include "Rendering/rendering_device.h"
#include <set>
#include "GLFW/glfw3native.h"

namespace mortal {

	void RenderingDevice::SetDevice(MortalWindowType* window)
	{
        vk::Result result;
        if constexpr (EnableValidtion) {
            s_LayerNames.push_back("VK_LAYER_KHRONOS_validation");
            CheckValidtionLayer();
        }
        s_DeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        //Set Vulkan Instance
        {
            vk::ApplicationInfo appInfo("Mortal", 1, "Mortal Engine", 1, VK_API_VERSION_1_3);

            auto extensions = GetRequireExtensions();
            vk::InstanceCreateInfo createInfo({}, &appInfo, s_LayerNames, extensions);
            
            m_Instance = vk::createInstance(createInfo);

            SetDebugCallBack();
        }

        //set surface , This section is platform-specific
        {
            if constexpr (Mortal_PlatformID == 1) {
                //in Windows
               
                vk::Win32SurfaceCreateInfoKHR createInfo({}, GetModuleHandle(nullptr), glfwGetWin32Window(window));
                m_Surface =  m_Instance.createWin32SurfaceKHR(createInfo);
            }
        }

        uint32_t queueFamilyIndex_GraphicPresent = 0;
        uint32_t queueFamilyIndex_Compute = 0;
        //Get Physical Device and queue family index
        {
            ChooseSuitablePhysicalDevice();

            uint32_t queueFamilyCount = 0;
            std::vector<vk::QueueFamilyProperties> properties = m_PhysicalDevice.getQueueFamilyProperties();
            
            uint32_t temp = 0;
            for (auto& property : properties) {
                if (property.queueFlags & vk::QueueFlagBits::eGraphics) {
                    //m_PhysicalDevice.getSurfaceSupportKHR();
                    queueFamilyIndex_GraphicPresent = temp;
                }
                //if you want to use compute shader
                if constexpr (EnableCompute) {
                    if (property.queueFlags & vk::QueueFlagBits::eCompute) {
                        queueFamilyIndex_Compute = temp;
                    }
                }
                temp++;
            }
        }
        //Set Queue Index
        {
            m_Queues.GraphicPresenQueueFamilyIndex = queueFamilyIndex_GraphicPresent;
            //compute shader enable
            if constexpr (EnableCompute) {
                m_Queues.ComputeQueueFamilyIndex = queueFamilyIndex_Compute;
            }
        }

        //Create Logic Device and Get Queue
        {
            std::vector<float> queuePriorites{ 1.0f };
            vk::DeviceQueueCreateInfo queueCreateInfo({}, m_Queues.GraphicPresenQueueFamilyIndex.value(), queuePriorites);

            auto feature = EnablePhysicalFeature();
            vk::DeviceCreateInfo createInfo({}, queueCreateInfo, s_LayerNames, s_DeviceExtensions, &feature);

            m_LogicDevice =  m_PhysicalDevice.createDevice(createInfo);

            m_Queues.GraphicPresentQueue = m_LogicDevice.getQueue(m_Queues.GraphicPresenQueueFamilyIndex.value(), 0);
            if constexpr (EnableCompute) {
                m_Queues.ComputeQueue = m_LogicDevice.getQueue(m_Queues.ComputeQueueFamilyIndex.value(), 0);
            }
        }
	}

    void RenderingDevice::ClearUpDevice()
    {
        m_LogicDevice.destroy();
        this->GetAndExecuteFunction<PFN_vkDestroyDebugUtilsMessengerEXT>("vkDestroyDebugUtilsMessengerEXT", callback, nullptr);
        m_Instance.destroySurfaceKHR(m_Surface);
        m_Instance.destroy();
    }

    vk::PhysicalDeviceFeatures RenderingDevice::EnablePhysicalFeature()
    {
        vk::PhysicalDeviceFeatures features;
        features.samplerAnisotropy = VK_TRUE;
        features.sampleRateShading = VK_TRUE;
        return features;
    }

    void RenderingDevice::SetDebugCallBack()
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

    //create instance
    std::vector<const char*> RenderingDevice::GetRequireExtensions()
    {
        uint32_t extensionsCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionsCount);
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + extensionsCount);
        if constexpr (EnableValidtion) {
            extensions.push_back("VK_EXT_debug_utils");
        }
        return extensions;
    }

//physical device
    vk::PhysicalDeviceProperties RenderingDevice::GetPhysicalDevicePropertires()
    {
        return m_PhysicalDevice.getProperties();
    }

    vk::PhysicalDeviceFeatures RenderingDevice::GetPhysicalDeviceFeature()
    {
        return m_PhysicalDevice.getFeatures();
    }


    vk::Device& RenderingDevice::GetDevice()
    {
        return m_LogicDevice;
    }

    RenderingQueue& RenderingDevice::GetRenderingQueue()
    {
        return m_Queues;
    }

    void RenderingDevice::ChooseSuitablePhysicalDevice()
    {
        std::vector<vk::PhysicalDevice> pDevices = m_Instance.enumeratePhysicalDevices();

        for (auto& pDevice : pDevices) {
            vk::PhysicalDeviceProperties properties = pDevice.getProperties();
            vk::PhysicalDeviceFeatures features = pDevice.getFeatures();
            if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu && features.geometryShader == VK_TRUE) {
                m_PhysicalDevice = pDevice;
                auto extensionProperties = m_PhysicalDevice.enumerateDeviceExtensionProperties();
                
                std::set<std::string_view> needEnableExtensions(s_DeviceExtensions.begin(), s_DeviceExtensions.end());
                for(auto& extensionProperty : extensionProperties) {
                    needEnableExtensions.erase(extensionProperty.extensionName);
                }
                if (!needEnableExtensions.empty()) {
                    throw std::runtime_error("don't have suitable physical device");
                }
                break;
            }
        }
    }

//validation layer set
    bool RenderingDevice::CheckValidtionLayer()
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

    VKAPI_ATTR VkBool32 VKAPI_CALL RenderingDevice::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
        VkDebugUtilsMessageTypeFlagsEXT messageType, 
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
        void* pUserData)
    {
        std::cerr << "validation layer: \n" << pCallbackData->pMessage << "\n\n";
        return VK_FALSE;
    }

}
