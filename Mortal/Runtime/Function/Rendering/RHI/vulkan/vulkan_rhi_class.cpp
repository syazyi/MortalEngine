#include "vulkan_rhi_class.h"

#include <set>
namespace mortal
{
    namespace rhi
    {
//-----------------------------------------API--------------------------------------------
        RenderInstance* RHI_Vulkan::CreateInstance(const CreateInstanceDescriptor *desc)
        {
             auto* ri = CreateInstance_Vulkan(desc);
             return ri;
        }

        void RHI_Vulkan::FreeInstance(const RenderInstance *ri)
        {
            assert(ri != nullptr);
            FreeInstance_Vulkan(ri);
        }

        std::vector<RenderPhysicalDevice*> RHI_Vulkan::EnumPhysicalDevice(RenderInstance* ri)
        {
            return EnumPhysicalDevice_Vulkan(ri);
        }

        RenderDevice* RHI_Vulkan::CreateDevice(RenderPhysicalDevice* rpd, CreateDeviceDesciptor d_desc)
        {
            return CreateDevice_Vulkan(rpd, d_desc);
        }

        void RHI_Vulkan::FreeDevice(RenderDevice* rd)
        {

        }



//-----------------------------------------Implement--------------------------------------------
    //Instance
        RenderInstance *RHI_Vulkan::CreateInstance_Vulkan(const CreateInstanceDescriptor *desc)
        {
            //prepare createinfo
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

            //enum all need physical device
            std::vector<vk::PhysicalDevice> pDevices = riv->instance.enumeratePhysicalDevices();
            for (auto& pd : pDevices) {
                RenderPhysicalDevice_Vulkan temp;
                temp.instance = riv;
                temp.physical_device = pd;
                riv->physical_devices.emplace_back(temp);
            }
            return reinterpret_cast<RenderInstance*>(riv);
        }

        void RHI_Vulkan::FreeInstance_Vulkan(const RenderInstance *ri)
        {
            auto riv = reinterpret_cast<const RenderInstance_Vulkan*>(ri);
            if constexpr (EnableValidtion) {
                GetAndExecuteFunction<PFN_vkDestroyDebugUtilsMessengerEXT>("vkDestroyDebugUtilsMessengerEXT", riv->instance, riv->callback_vulkan, nullptr);
            }
            riv->instance.destroy();
        }
        //End of Instance

        //Physical
        std::vector<RenderPhysicalDevice*> RHI_Vulkan::EnumPhysicalDevice_Vulkan(RenderInstance* ri)
        {
            auto* riv = reinterpret_cast<RenderInstance_Vulkan*>(ri);
            std::vector<RenderPhysicalDevice*> rpd;
            for (auto& pd : riv->physical_devices) {
                rpd.emplace_back(reinterpret_cast<RenderPhysicalDevice*>(&pd));
            }
            return rpd;
        }
        //End of Physical

        //Device
        RenderDevice* RHI_Vulkan::CreateDevice_Vulkan(RenderPhysicalDevice* rpd, CreateDeviceDesciptor d_desc)
        {
            //Assume it have a best PhysicalDevice
            //and Queue Family store in PhysicalStruct
                    //Create Logic Device and Get Queue
            auto* rpdv = reinterpret_cast<RenderPhysicalDevice_Vulkan*>(rpd);
            std::set<uint32_t> queueIndices
            { 
                rpdv->queue_families.GraphicQueueFamilyIndex.value(),
                rpdv->queue_families.PresentQueueFamilyIndex.value(), 
                rpdv->queue_families.ComputeQueueFamilyIndex.value() 
            };
            std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
            for (auto& queueIndex : queueIndices) {
                float queuePriorites = 1.0f;
                vk::DeviceQueueCreateInfo createInfo({}, queueIndex, 1, &queuePriorites);
                queueCreateInfos.push_back(createInfo);
            }
            //auto feature = EnablePhysicalFeature();
            //vk::DeviceCreateInfo createInfo({}, queueCreateInfos, s_LayerNames, s_DeviceExtensions, &feature);

            auto* rd = new RenderDevice_Vulkan;

            //rd->device = rpdv->physical_device.createDevice(createInfo);

            //rpdv->queue_families.GraphicQueue = rd->device.getQueue(rpdv->queue_families.GraphicQueueFamilyIndex.value(), 0);
            //rpdv->queue_families.PresentQueue = rd->device.getQueue(rpdv->queue_families.PresentQueueFamilyIndex.value(), 0);
            //rpdv->queue_families.ComputeQueue = rd->device.getQueue(rpdv->queue_families.ComputeQueueFamilyIndex.value(), 0);
            return rd;
        }
        void RHI_Vulkan::FreeDevice_Vulkan(RenderDevice* rd)
        {

        }
        //End of Device
    } // namespace rhi
    
} // namespace mortal

