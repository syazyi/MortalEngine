#include "api.h"
#include "Rendering/rendering.h"
#include "Rendering/RHI/vulkan/proctable_vulkan.h"
namespace mortal
{
    namespace renderAPI
    {
        RenderInstance* CreateInstance(CreateInstanceDescriptor* desc)
        {
            const ProcTable* ptv = nullptr;
            if (desc->api_type == RenderAPIType::eVulkan) {
                ptv = GetProcTableVulkan();
            }
            else {
                throw "Currently, only Vulkan is supported";
            }

             auto* ri = ptv->create_instance(desc);
             ri->proc_table = ptv;
             return ri;
        }

        void FreeInstance(const RenderInstance* ri)
        {
            assert(ri != nullptr);
            assert(ri->proc_table != nullptr);

            ri->proc_table->free_instance(ri);
        }
    } // namespace renderAPI
    

} // namespace mortal
