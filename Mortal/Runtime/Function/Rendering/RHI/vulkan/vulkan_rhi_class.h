#pragma once
#include "Rendering/rendering.h"
#include "vulkan_utility.h"
#include "Rendering/RHI/rhi_class.h"
namespace mortal
{
    namespace rhi
    {
        struct RenderPhysicalDevice_Vulkan;

        //Instance
        struct RenderInstance_Vulkan : public RenderInstance
        {
            vk::Instance instance;
            std::vector<RenderPhysicalDevice_Vulkan> physical_devices;
            VkDebugUtilsMessengerEXT callback_vulkan;
        };

        struct RenderDevice_Vulkan : public RenderDevice
        {
            vk::Device device;
        };

        struct RenderPhysicalDevice_Vulkan : public RenderPhysicalDevice
        {
            vk::PhysicalDevice physical_device;
            RenderingQueue_Vulkan queue_families;
        };

        class RHI_Vulkan : public RHIBase
        {
        public:    
            ~RHI_Vulkan() {};
            //Instance
            virtual RenderInstance* CreateInstance(const CreateInstanceDescriptor* desc) override;
            virtual void FreeInstance(const RenderInstance* ri) override;
            //PhysicalDevice
            virtual std::vector<RenderPhysicalDevice*> EnumPhysicalDevice(RenderInstance* ri)  override;
            //Device
            virtual RenderDevice* CreateDevice(RenderPhysicalDevice* rpd, CreateDeviceDesciptor d_desc) override;
            virtual void FreeDevice(RenderDevice* rd) override;
        private:
        //Implement
            //Instance
            RenderInstance* CreateInstance_Vulkan(const CreateInstanceDescriptor* desc);
            void FreeInstance_Vulkan(const RenderInstance* ri);
            //Physicaldevice
            std::vector<RenderPhysicalDevice*> EnumPhysicalDevice_Vulkan(RenderInstance* ri);
            //Device
            RenderDevice* CreateDevice_Vulkan(RenderPhysicalDevice* rpd, CreateDeviceDesciptor d_desc);
            void FreeDevice_Vulkan(RenderDevice* rd);
        };

    } // namespace RHI
    
} // namespace mortal
