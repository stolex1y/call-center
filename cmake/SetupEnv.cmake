include_guard(GLOBAL)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}(${CMAKE_CXX_COMPILER})")

#warnings
add_compile_options("-Werror" "-Wall" "-Wextra" "-Wpedantic")

enable_testing()