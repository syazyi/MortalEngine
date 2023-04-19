cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(MORTAL_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
project(Mortal)
add_subdirectory(Editor)

add_subdirectory(Mortal)