#pragma once
#include "Rendering/rendering.h"
#include "Rendering/RHI/rhi_class.h"
namespace mortal
{
    namespace rhi
    {
        //Instance
        struct RenderInstance_Vulkan : public RenderInstance
        {
            vk::Instance instance;
            std::vector<RenderPhysicalDevice_Vulkan> physical_devices;
            VkDebugUtilsMessengerEXT callback_vulkan;
        };

        struct RenderPhysicalDevice_Vulkan : public RenderPhysicalDevice
        {
            vk::PhysicalDevice physical_device;
        };

        class RHI_Vulkan : public RHIBase
        {
        public:    
            virtual RenderInstance* CreateInstance(const CreateInstanceDescriptor* desc) override;
            virtual void FreeInstance(const RenderInstance* ri) override;
            virtual std::vector<RenderPhysicalDevice*> EnumPhysicalDevice(RenderInstance* ri)  override;
        private:
        //Implement
            RenderInstance* CreateInstance_Vulkan(const CreateInstanceDescriptor* desc);
            void FreeInstance_Vulkan(const RenderInstance* ri);
            std::vector<RenderPhysicalDevice*> EnumPhysicalDevice_Vulkan(RenderInstance* ri);
        };

    } // namespace RHI
    
} // namespace mortal
