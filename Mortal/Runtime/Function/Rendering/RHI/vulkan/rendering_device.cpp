#include "rendering_device.h"
#include <set>
#include "GLFW/glfw3native.h"

namespace mortal {

	void RenderingDevice::SetDevice(vk::Instance& instance, vk::SurfaceKHR& surface)
	{
        vk::Result result;
        s_DeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        m_InstanceRef = &instance;
        uint32_t queueFamilyIndex_GraphicPresent = 0;
        uint32_t queueFamilyIndex_Compute = 0;
        //Get Physical Device and queue family index
        {
            ChooseSuitablePhysicalDevice(instance);

            //uint32_t queueFamilyCount = 0;
            std::vector<vk::QueueFamilyProperties> properties = m_PhysicalDevice.getQueueFamilyProperties();
            
            uint32_t temp = 0;
            for (auto& property : properties) {
                if (property.queueFlags & vk::QueueFlagBits::eGraphics) {
                    if (m_PhysicalDevice.getSurfaceSupportKHR(temp, surface)) {
                        queueFamilyIndex_GraphicPresent = temp;
                    }
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
            //Set same queue family
            m_Queues.GraphicQueueFamilyIndex = queueFamilyIndex_GraphicPresent;
            m_Queues.PresentQueueFamilyIndex = queueFamilyIndex_GraphicPresent;
            //compute shader enable
            if constexpr (EnableCompute) {
                m_Queues.ComputeQueueFamilyIndex = queueFamilyIndex_Compute;
            }
        }

        //Create Logic Device and Get Queue
        {
            std::set<uint32_t> queueIndices{m_Queues.GraphicQueueFamilyIndex.value(),
               m_Queues.PresentQueueFamilyIndex.value()};
            if constexpr (EnableCompute) {
                queueIndices.emplace(m_Queues.ComputeQueueFamilyIndex.value());
            }
            std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
            for (auto& queueIndex : queueIndices) {
                float queuePriorites = 1.0f;
                vk::DeviceQueueCreateInfo createInfo({}, queueIndex, 1, &queuePriorites);
                queueCreateInfos.push_back(createInfo);
            }
            auto feature = EnablePhysicalFeature();
            vk::DeviceCreateInfo createInfo({}, queueCreateInfos, s_LayerNames, s_DeviceExtensions, &feature);

            m_LogicDevice =  m_PhysicalDevice.createDevice(createInfo);

            m_Queues.GraphicQueue = m_LogicDevice.getQueue(m_Queues.GraphicQueueFamilyIndex.value(), 0);
            m_Queues.PresentQueue = m_LogicDevice.getQueue(m_Queues.PresentQueueFamilyIndex.value(), 0);
            if constexpr (EnableCompute) {
                m_Queues.ComputeQueue = m_LogicDevice.getQueue(m_Queues.ComputeQueueFamilyIndex.value(), 0);
            }
        }
	}

    void RenderingDevice::ClearUp()
    {
        m_LogicDevice.destroy();
    }

    vk::PhysicalDeviceFeatures RenderingDevice::EnablePhysicalFeature()
    {
        vk::PhysicalDeviceFeatures features;
        features.samplerAnisotropy = VK_TRUE;
        features.sampleRateShading = VK_TRUE;
        return features;
    }

//physical device
    vk::PhysicalDeviceProperties RenderingDevice::GetPhysicalDeviceProperties()
    {
        return m_PhysicalDevice.getProperties();
    }

    vk::FormatProperties RenderingDevice::GetPhysicalDeviceFormatProperties(vk::Format format)
    {
        return m_PhysicalDevice.getFormatProperties(format);
    }

    vk::PhysicalDeviceFeatures RenderingDevice::GetPhysicalDeviceFeature()
    {
        return m_PhysicalDevice.getFeatures();
    }


    vk::Device& RenderingDevice::GetDevice()
    {
        return m_LogicDevice;
    }

    vk::PhysicalDevice& RenderingDevice::GetPhysicalDevice()
    {
        return m_PhysicalDevice;
    }

    RenderingQueue& RenderingDevice::GetRenderingQueue()
    {
        return m_Queues;
    }

    vk::Instance& RenderingDevice::GetInstanceRef()
    {
        return *m_InstanceRef;
    }

    std::pair<vk::Format, bool> RenderingDevice::FindSupportDepthFormat(const std::vector<vk::Format>& formats, vk::ImageTiling imageTiling, vk::FormatFeatureFlags features)
    {
        for (auto& format : formats) {
            auto formatPro =  GetPhysicalDeviceFormatProperties(format);
            switch (imageTiling)
            {
            case vk::ImageTiling::eOptimal:
                if ((formatPro.optimalTilingFeatures & features) == features) {
                    if (format == vk::Format::eD32Sfloat) {
                        return {format, false};
                    }
                    return { format, true };
                }
                break;
            case vk::ImageTiling::eLinear:
                if ((formatPro.linearTilingFeatures & features) == features) {
                    if (format == vk::Format::eD32Sfloat) {
                        return { format, false };
                    }
                    return { format, true };
                }
                break;
            case vk::ImageTiling::eDrmFormatModifierEXT:
                break;
            }
            throw "Error";
        }
    }

    void RenderingDevice::ChooseSuitablePhysicalDevice(vk::Instance& instance)
    {
        std::vector<vk::PhysicalDevice> pDevices = instance.enumeratePhysicalDevices();

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

    uint32_t RenderingDevice::FindMemoryIndex(const std::vector<vk::MemoryRequirements>& requirements, vk::MemoryPropertyFlags flags) {
        vk::PhysicalDeviceMemoryProperties property = m_PhysicalDevice.getMemoryProperties();
        uint32_t index = 0;
        for (; index < property.memoryTypeCount; index++) {
            uint32_t value = 0xFFFFFFFF;
            for (auto& require : requirements) {
                value = (require.memoryTypeBits & (1 << index)) & value;
            }
            if (value && property.memoryTypes[index].propertyFlags & flags) {
                return index;
            }
        }
        if (index == property.memoryTypeCount) {
            MORTAL_LOG_ERROR("Some Buffers don't suitable be allocated in the same memory");
            throw std::runtime_error("Error");
        }
        return UINT32_MAX;
    }
}
