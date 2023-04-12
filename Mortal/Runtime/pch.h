#pragma once

    #include <iostream>
    #include <memory>
    #include <algorithm>
    #include <utility>
    #include <functional>   

    #include <string> 
    #include <sstream>
    #include <vector>
    #include <unordered_map>
    #include <unordered_set>

    #ifdef MORTAL_PLATFORM_WINDOWS
        #define VK_USE_PLATFORM_WIN32_KHR
        #include"vulkan/vulkan.hpp"
        #include <Windows.h>

        #define GLFW_INCLUDE_VULKAN
        #include "GLFW/glfw3.h"
        #define GLFW_EXPOSE_NATIVE_WIN32
        #include "GLFW/glfw3native.h"        
    #endif




