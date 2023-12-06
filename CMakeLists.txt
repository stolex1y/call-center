cmake_minimum_required(VERSION 3.27)
file(STRINGS "version.txt" PROJECT_VERSION)
project("call-center" VERSION "${PROJECT_VERSION}" LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(SetupEnv)

add_subdirectory(src bin)
add_subdirectory(test)
