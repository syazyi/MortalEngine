#pragma once

namespace mortal
{
    namespace rhi
    {
        //Compilation parameter settings
        constexpr uint8_t PhysicalDeviceNums = 64;
        //End of Compilation parameter settings

        enum class RenderAPIType
        {
            eVulkan, 
            eDirectX12,
            eMetal
        };

        enum class QueueType 
        {
            eGrphics = 0,
            eCompute = 1,
            eTransfer = 2
        };

        //Object
        struct RenderInstance{};
        
        struct RenderPhysicalDevice
        {
            RenderInstance* instance;
        };

        struct RenderDevice
        {
            RenderPhysicalDevice* physical_deivce;
        };
        //Descriptors
        struct CreateInstanceDescriptor
        {
            RenderAPIType api_type;
            bool enable_validtion;
        };
        
        struct CreateDeviceQueueGroupDescriptor
        {
            QueueType queue_type;
            uint32_t queue_count;
        };

        struct CreateDeviceDesciptor
        {
            CreateDeviceQueueGroupDescriptor* queue_group_des;
            uint32_t queue_group_count;
        };

        class RHIBase {
        public:
            virtual ~RHIBase() = 0;

            //Instance
            virtual RenderInstance* CreateInstance(const CreateInstanceDescriptor* desc) = 0;
            virtual void FreeInstance(const RenderInstance* ri) = 0;
            //End of Instance

            //Physical
            virtual std::vector<RenderPhysicalDevice*> EnumPhysicalDevice(RenderInstance* ri) = 0;
            //End of Physical

            //Device
            virtual RenderDevice* CreateDevice(RenderPhysicalDevice* rpd, CreateDeviceDesciptor d_desc) = 0;
            virtual void FreeDevice(RenderDevice* rd) = 0;
            //End of Device

        };
    } // namespace rhi
} // namespace mortal
