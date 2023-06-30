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

        struct RenderInstance
        {
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

            ProcTable() = default;
            const ProcCreateInstance create_instance;
            const ProcFreeInstance free_instance;
        };
        
        //example
        /*struct RHIContext
        {
            // window info inclue handle
             
            //swapchian ...
        };
        */

        // api
        RenderInstance* CreateInstance(const CreateInstanceDescriptor* desc);
        void FreeInstance(const RenderInstance* ri);

    } // namespace renderAPI
} // namespace mortal
