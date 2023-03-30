#pragma once 
#include "Mortal.h"
#include "vulkan/vulkan.h"
#include "Layer/Layer.h"

namespace mortal
{
    struct RenderingSystemInfo
    {
        VkInstance instance {VK_NULL_HANDLE};
        VkAllocationCallbacks* allocation{ nullptr };
        VkPhysicalDevice physical_device{ VK_NULL_HANDLE };
        VkDevice logic_device { VK_NULL_HANDLE };
        VkQueue queue{VK_NULL_HANDLE};
    };
    
    void CheckVulkanResult(VkResult& result) {
        if (result != VK_SUCCESS) {
            std::runtime_error("failed to pass check");
        }
    }
    
    class MORTAL_API RenderingSystem : public Layer{
    public:
        ~RenderingSystem();
        RenderingSystem(const RenderingSystem&) = delete;
        RenderingSystem& operator=(const RenderingSystem&) = delete;

        static RenderingSystem* GetInstance(){
            static RenderingSystem* instance = new RenderingSystem;
            return instance;
        }    

        void SetUpVulkan();        
        void ClearUpVulkan();

        inline RenderingSystemInfo& GetVulkanInfo(){
            return info;
        }

    private:
        RenderingSystemInfo info;
        RenderingSystem();
    };


} // namespace mortal
