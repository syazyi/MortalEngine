#include "rendering_system.h"

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
        VkResult result;
    //Set Vulkan Instance
        {   
            VkApplicationInfo appInfo{};
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = "Mortal";
            appInfo.applicationVersion = VK_VERSION_1_0;
            appInfo.pEngineName = "Mortal Engine";
            appInfo.engineVersion = VK_VERSION_1_0;
            appInfo.apiVersion = VK_VERSION_1_3;


            VkInstanceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;

            //if define vaildtion,set this
            createInfo.enabledLayerCount = 0;
            createInfo.ppEnabledLayerNames = nullptr;

            //get Glfw extension
            uint32_t count = 0;
            auto names = glfwGetRequiredInstanceExtensions(&count);

            createInfo.enabledExtensionCount = count;
            createInfo.ppEnabledExtensionNames = names;

            result = vkCreateInstance(&createInfo, info.allocation, &info.instance);

            CheckVulkanResult(result);

        }

        uint32_t queueFamilyIndex = 0;
    //Get Physical Device
        {
            uint32_t count = 0;
            vkEnumeratePhysicalDevices(info.instance, &count, nullptr);
            std::vector<VkPhysicalDevice> pDevices(count);
            vkEnumeratePhysicalDevices(info.instance, &count, pDevices.data());

            for (auto& pDevice : pDevices) {
                VkPhysicalDeviceProperties properties;
                vkGetPhysicalDeviceProperties(pDevice, &properties);
                VkPhysicalDeviceFeatures features;
                vkGetPhysicalDeviceFeatures(pDevice, &features);
                if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && features.geometryShader == VK_TRUE) {
                    info.physical_device = pDevice;
                    break;
                }
            }

            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(info.physical_device, &queueFamilyCount, nullptr);
            std::vector<VkQueueFamilyProperties> properties(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(info.physical_device, &queueFamilyCount, properties.data());
            for (auto& property : properties) {
                if (property.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT) {
                    break;
                }
                queueFamilyIndex++;
            }

        }

    //Create Logic Device
        {
            VkDeviceQueueCreateInfo qCreateInfo{};
            qCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            float Priorities = 1.0f;
            qCreateInfo.pQueuePriorities = &Priorities;
            qCreateInfo.queueCount = 1;
            qCreateInfo.queueFamilyIndex = queueFamilyIndex;

            VkDeviceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            
            createInfo.queueCreateInfoCount = 1;
            createInfo.pQueueCreateInfos = &qCreateInfo;
            
            createInfo.enabledLayerCount = 0;
            createInfo.ppEnabledLayerNames = nullptr;

            std::vector<const char*> extensionNames{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
            createInfo.enabledExtensionCount = 1;
            createInfo.ppEnabledExtensionNames = extensionNames.data();

            createInfo.pEnabledFeatures = nullptr;

            result = vkCreateDevice(info.physical_device, &createInfo, info.allocation, &info.logic_device);
            CheckVulkanResult(result);

            vkGetDeviceQueue(info.logic_device, queueFamilyIndex, 0, &info.queue);
        }
    }

    void RenderingSystem::ClearUpVulkan()
    {
        vkDestroyDevice(info.logic_device, info.allocation);
        vkDestroyInstance(info.instance, info.allocation);
    }

} // namespace mortal
