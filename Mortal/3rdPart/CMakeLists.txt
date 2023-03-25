# Set spdlog
set_property(GLOBAL PROPERTY USE_FOLDERS On)
add_subdirectory(spdlog)
set_property(TARGET spdlog PROPERTY FOLDER "3rdPart")

# Set Glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)