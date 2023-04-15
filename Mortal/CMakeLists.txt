set(THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdPart")

set(SHADER_COMPILE_TARGET MortalShaderCompile)

if(WIN32)
    set(glslc_executable ${THIRD_PARTY_DIR}/VulkanSDK/bin/glslc.exe)
endif()

add_subdirectory(3rdPart)
add_subdirectory(Runtime)
add_subdirectory(Shader)