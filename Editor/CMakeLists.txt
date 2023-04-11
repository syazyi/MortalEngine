set(EXENAME Editor)
if(MSVC)
    add_definitions(-DMORTAL_PLATFORM_WINDOWS)
endif(MSVC)

file(GLOB_RECURSE HEADER_EDITOR_FILE "*.h" )
file(GLOB_RECURSE SOURCE_EDITOR_FILE "*.cpp" )

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADER_EDITOR_FILE} ${SOURCE_EDITOR_FILE})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
add_executable(${EXENAME} ${HEADER_EDITOR_FILE} ${SOURCE_EDITOR_FILE})
target_link_libraries(${EXENAME} PUBLIC Mortal)
target_link_directories(${EXENAME} PUBLIC ${PROJECT_SOURCE_DIR}/bin) 
#target_include_directories(${EXENAME} PUBLIC ../Mortal)