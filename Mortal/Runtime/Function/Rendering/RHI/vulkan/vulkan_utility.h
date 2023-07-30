#pragma once 
#include <optional>
#include "Rendering/rendering.h"
namespace mortal
{
    namespace rhi
    {
        VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
     
        //Instance
        struct VkInstanceUtil {
            VkInstanceUtil()
            {
                if constexpr (EnableValidtion)
                {
                    instance_layers.emplace_back("VK_LAYER_KHRONOS_validation");
                    instance_extensions.emplace_back("VK_EXT_debug_utils");

                    debugUtilsCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
                    debugUtilsCreateInfo.messageSeverity = VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                        VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                        VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
                    debugUtilsCreateInfo.messageType = VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                        VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                        VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
                    debugUtilsCreateInfo.pfnUserCallback = &debugCallback;
                }
            }
            std::vector<const char*> instance_layers;
            std::vector<const char*> instance_extensions;
            VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo;
        };


        template<typename T>
        T GetFunction(const char* funcName, vk::Instance instance) {
            auto func = reinterpret_cast<T>(instance.getProcAddr(funcName));
            if (!func) {
                throw "error";
            }
            return func;
        }

        template<typename T, typename... Args>
        void GetAndExecuteFunction(const char* funcName, vk::Instance instance, Args&&... args) {
            auto func = reinterpret_cast<T>(instance.getProcAddr(funcName));
            func(instance, std::forward<Args>(args)...);
        }
        //End of Instance 


        //Physical 
        struct RenderingQueue_Vulkan
        {
            std::optional<vk::Queue> GraphicQueue;
            std::optional<uint32_t> GraphicQueueFamilyIndex;

            std::optional<vk::Queue> PresentQueue;
            std::optional<uint32_t> PresentQueueFamilyIndex;

            std::optional<vk::Queue> ComputeQueue;
            std::optional<uint32_t> ComputeQueueFamilyIndex;

            bool HasGraphicPresentQueue();
        };

    } // namespace rhi
    
} // namespace mortal
