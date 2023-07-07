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

        //Object
        struct RenderInstance
        {
        };
        
        struct RenderPhysicalDevice
        {
            RenderInstance* instance;
        };

        //Descriptors
        struct CreateInstanceDescriptor
        {
            RenderAPIType api_type;
            bool enable_validtion;
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
        };
    } // namespace rhi
    

} // namespace mortal
