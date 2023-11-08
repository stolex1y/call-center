set(OBJ_LIB_TARGET callcenter)
set(BOOTSTRAP_TARGET "${TARGET}_bootstrap")
set(STATIC_LIB_TARGET "${TARGET}_static")

find_package(Boost REQUIRED)

add_library(${OBJ_LIB_TARGET} OBJECT
        call_center_service.cpp
)
target_include_directories(${OBJ_LIB_TARGET} PRIVATE
        include
        Boost::boost
)
target_include_directories(${OBJ_LIB_TARGET} PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
target_link_libraries(${OBJ_LIB_TARGET} PRIVATE ${Boost_LIBRARIES})

add_library(${STATIC_LIB_TARGET} STATIC)
target_link_libraries(${STATIC_LIB_TARGET} ${OBJ_LIB_TARGET})

add_executable(${BOOTSTRAP_TARGET} main.cpp)
target_link_libraries(${BOOTSTRAP_TARGET} ${OBJ_LIB_TARGET})
