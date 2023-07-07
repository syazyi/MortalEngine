#include "vulkan_rhi_class.h"
#include "vulkan_utility.h"
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

    } // namespace rhi
    
} // namespace mortal

