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



    constexpr bool EnableCompute = false;

    constexpr uint8_t MaxFrameInFlight = 2;

    /*
     static void CheckVulkanResult(vk::Result& result, vk::Result condition, const char* message) {
        if (result != condition) {
             throw std::runtime_error(message);
        }
    }
    */

    template<typename T, typename... Args>
    vk::Result LoadFunction() {
        auto func = reinterpret_cast<T>()
    }

    static std::vector<const char*> s_LayerNames;
} // namespace mortal
