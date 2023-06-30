#pragma once

#ifdef WIN32
constexpr uint8_t Mortal_PlatformID = 1;
#endif // WIN32

#include "Core.h"
#include "vulkan/vulkan.hpp"
namespace mortal
{
#ifdef NDEBUG
    constexpr bool EnableValidtion = false;
#else
    constexpr bool EnableValidtion = true;
#endif // NDEBUG

    constexpr bool IsRenderAPIVulkan = true;

    constexpr bool EnableCompute = true;

    constexpr uint8_t MaxFrameInFlight = 2;

    static std::vector<const char*> s_LayerNames;
    /*
     static void CheckVulkanResult(vk::Result& result, vk::Result condition, const char* message) {
        if (result != condition) {
             throw std::runtime_error(message);
        }
    }
    */
} // namespace mortal
