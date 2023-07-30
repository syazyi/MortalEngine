#include "vulkan_utility.h"

namespace mortal 
{
    namespace rhi 
    {
        VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
        {
            std::cerr << "validation layer: \n" << pCallbackData->pMessage << "\n\n";
            return VK_FALSE;
        }


        bool RenderingQueue_Vulkan::HasGraphicPresentQueue()
        {
             return GraphicQueue.has_value() && GraphicQueueFamilyIndex.has_value() && PresentQueue.has_value() && PresentQueueFamilyIndex.has_value();
        }

    }
}

