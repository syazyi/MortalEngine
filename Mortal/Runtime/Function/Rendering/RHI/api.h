#pragma once

namespace mortal
{
    namespace renderAPI
    {
        //forward declare
        struct ProcTable;

        enum class RenderAPIType
        {
            eVulkan, 
            eDirectX12,
            eMetal
        };

        //Object
        struct RenderInstance
        {
            const ProcTable* proc_table;
        };
        
        struct RenderPhysicalDevice
        {
            RenderInstance* instance;
            const ProcTable* proc_table;
        };

        //Descriptors
        struct CreateInstanceDescriptor
        {
            RenderAPIType api_type;
            bool enable_validtion;
        };

        //API proc
        //Proctable
        struct ProcTable
        {
            using ProcCreateInstance = RenderInstance*(*)(const CreateInstanceDescriptor*);
            using ProcFreeInstance = void(*)(const RenderInstance*);

            using ProcEnumPhysicalDevice = void(*)(RenderInstance*, const RenderPhysicalDevice*);
            ProcTable() = default;
            //Instance
            const ProcCreateInstance create_instance;
            const ProcFreeInstance free_instance;
            //End of Instance

            //Physical(adapter)
            const ProcEnumPhysicalDevice enum_physical_device;
            //End of Physical(adapter)

            //Device
            //To do 
            //End of Device
        };
        
        //example
        /*struct RHIContext
        {
            // window info inclue handle
             
            //swapchian ...
        };
        */

        // api
        //Instance
        RenderInstance* CreateInstance(const CreateInstanceDescriptor* desc);
        void FreeInstance(const RenderInstance* ri);
        //End of Instance

        //Physical(adapter)
        void EnumPhysicalDevice(RenderInstance* ri, const RenderPhysicalDevice* rpd);
        //End of Physical(adapter)

    } // namespace renderAPI
} // namespace mortal
