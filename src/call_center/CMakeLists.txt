set(OBJ_LIB_TARGET callcenter)
set(RUNNABLE_TARGET "${OBJ_LIB_TARGET}_runnable")
set(STATIC_LIB_TARGET "${OBJ_LIB_TARGET}_static")

find_package(Boost COMPONENTS program_options log log_setup json REQUIRED)

add_library(${OBJ_LIB_TARGET} OBJECT
        log/logger.cc
        log/logger.h
        call_center.cc
        call_center.h
        journal.cc
        journal.h
        configuration.cc
        configuration.h
        operator.cc
        operator.h
        call_detailed_record.cc
        call_detailed_record.h
        data/call_repository.cc
        data/call_repository.h
        data/http_repository.cc
        data/http_repository.h
        data/http_server.cc
        data/http_server.h
        data/http.h
        data/http_connection.cc
        data/http_connection.h
        operator_set.cc
        operator_set.h
        call_queue.cc
        call_queue.h
        call_status.cc
        call_status.h
        data/call_request_dto.cc
        data/call_request_dto.h
        data/call_response_dto.cc
        data/call_response_dto.h
        core/task_manager.cc
        core/task_manager.h
        log/sink.cc
        log/sink.h
        log/severity_level.h
        log/attrs.h
        log/severity_level.cc
        core/containers/queue.h
        core/containers/concurrent_hash_set.h
        core/tasks.h
        core/utils/date_time.h
        core/utils/concepts.h
        log/logger_provider.cc
        log/logger_provider.h
        configuration_updater.cc
        configuration_updater.h
        core/containers/concurrent_hash_map.h
)

#warnings
target_compile_options(${OBJ_LIB_TARGET} PRIVATE "-Werror" "-Wall" "-Wextra" "-Wpedantic" "-Wno-error=maybe-uninitialized")
target_include_directories(${OBJ_LIB_TARGET} SYSTEM PUBLIC
        Boost::boost
)
target_include_directories(${OBJ_LIB_TARGET} PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
target_link_libraries(${OBJ_LIB_TARGET} PUBLIC
        ${Boost_LIBRARIES}
)
target_compile_definitions(${OBJ_LIB_TARGET} PUBLIC "BOOST_LOG_DYN_LINK")

add_library(${STATIC_LIB_TARGET} STATIC)
target_link_libraries(${STATIC_LIB_TARGET} ${OBJ_LIB_TARGET})

add_executable(${RUNNABLE_TARGET} main.cc)
target_link_libraries(${RUNNABLE_TARGET} ${OBJ_LIB_TARGET})

# clang-format
include(Format)
Format(${RUNNABLE_TARGET} .)

# copy config.json
add_custom_command(
        TARGET ${RUNNABLE_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/config.json
        ${CMAKE_CURRENT_BINARY_DIR}/config.json
)
