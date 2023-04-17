set(3RDPART_FILE_PATH "3rdPart")

# Set vulkan
set(vulkan_include ${CMAKE_CURRENT_SOURCE_DIR}/VulkanSDK/include)
set(vulkan_lib ${CMAKE_CURRENT_SOURCE_DIR}/VulkanSDK/lib/vulkan-1.lib)

# Set spdlog
set_property(GLOBAL PROPERTY USE_FOLDERS On)
add_subdirectory(spdlog)
set_property(TARGET spdlog PROPERTY FOLDER ${3RDPART_FILE_PATH})

# Set Glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)

# Set Glm
add_subdirectory(glm)

# Set Imgui
include(imgui.cmake)
set_target_properties(imgui PROPERTIES FOLDER ${3RDPART_FILE_PATH})