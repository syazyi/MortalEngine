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
        #include <Windows.h>
    #endif

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"