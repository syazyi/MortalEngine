set(LIBPROJECT_NAME Mortal)
set(MORTAL_PATH ${CMAKE_CURRENT_SOURCE_DIR})
if(MSVC)
    add_definitions(-DMORTAL_PLATFORM_WINDOWS -DMORTAL_BUILD_DLL)
endif()

file(GLOB_RECURSE HEADER_FILE "*.h" )
file(GLOB_RECURSE SOURCE_FILE "*.cpp" )

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADER_FILE} ${SOURCE_FILE})

add_library(${LIBPROJECT_NAME} SHARED ${HEADER_FILE} ${SOURCE_FILE})

#link 3rdLib
target_link_libraries(${LIBPROJECT_NAME} PUBLIC spdlog::spdlog)

#runtime path
target_include_directories(${LIBPROJECT_NAME} PUBLIC ${MORTAL_PATH})
#Core path
target_include_directories(${LIBPROJECT_NAME} PUBLIC ${MORTAL_PATH}/Core)
#3rd Library Path


#Set output path
set_target_properties(${LIBPROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

#set precompiled header
target_precompile_headers(${LIBPROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/pch.h)