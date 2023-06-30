#include "proctable_vulkan.h"
#include "Rendering/RHI/vulkan/vulkan_rhi.h"
namespace mortal
{
    namespace renderAPI
    {
        const ProcTable* mortal::renderAPI::GetProcTableVulkan()
        {
            static const ProcTable vulaknTable{
                &CreateInstance_Vulkan, 
                &FreeInstance_Vulkan
            };
            return &vulaknTable;
        }
    } // namespace renderAPI
} // namespace mortal
