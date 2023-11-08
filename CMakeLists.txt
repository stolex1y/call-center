cmake_minimum_required(VERSION 3.27)
project(call_center VERSION 1.0.0 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(SetupEnv)

add_subdirectory(src bin)
add_subdirectory(test)
