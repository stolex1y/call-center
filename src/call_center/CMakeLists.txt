set(OBJ_LIB_TARGET callcenter)
set(RUNNABLE_TARGET "${TARGET}_runnable")
set(STATIC_LIB_TARGET "${TARGET}_static")

find_package(Boost COMPONENTS program_options log log_setup REQUIRED)

add_library(${OBJ_LIB_TARGET} OBJECT
        log/logger.cc
        log/logger.h
)

#warnings
target_compile_options(${OBJ_LIB_TARGET} PRIVATE "-Werror" "-Wall" "-Wextra" "-Wpedantic")
target_include_directories(${OBJ_LIB_TARGET} PRIVATE
        include
        Boost::boost
)
target_include_directories(${OBJ_LIB_TARGET} PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
target_link_libraries(${OBJ_LIB_TARGET} PUBLIC
        ${Boost_LIBRARIES}
)

add_library(${STATIC_LIB_TARGET} STATIC)
target_link_libraries(${STATIC_LIB_TARGET} ${OBJ_LIB_TARGET})

add_executable(${RUNNABLE_TARGET} main.cc)
target_link_libraries(${RUNNABLE_TARGET} ${OBJ_LIB_TARGET})
