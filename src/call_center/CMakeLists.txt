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
        repository/call/call_repository.cc
        repository/call/call_repository.h
        core/http/http_repository.cc
        core/http/http_repository.h
        core/http/http_server.cc
        core/http/http_server.h
        core/http/http.h
        core/http/http_connection.cc
        core/http/http_connection.h
        operator_set.cc
        operator_set.h
        call_queue.cc
        call_queue.h
        call_status.cc
        call_status.h
        repository/call/call_request_dto.cc
        repository/call/call_request_dto.h
        repository/call/call_response_dto.cc
        repository/call/call_response_dto.h
        core/task_manager_impl.cc
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
        core/utils/uuids.h
        core/utils/uuids.cc
        core/task_manager_impl.h
        queueing_system/metrics/queueing_system_metrics.cc
        queueing_system/metrics/queueing_system_metrics.h
        queueing_system/metrics/metric.h
        queueing_system/metrics/request_metrics.cc
        queueing_system/metrics/request_metrics.h
        queueing_system/metrics/service_metrics.cc
        queueing_system/metrics/service_metrics.h
        queueing_system/server.h
        queueing_system/request.h
        queueing_system/request.cc
        queueing_system/server.cc
        repository/metrics/metrics_repository.cc
        repository/metrics/metrics_repository.h
        repository/metrics/metrics_response_dto.cc
        repository/metrics/metrics_response_dto.h
        core/utils/numbers.h
        core/clock_adapter.h
        core/clock_adapter.cc
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

#clang-tidy
#include(ClangTidy)
#AddClangTidy(${RUNNABLE_TARGET})

#iwyu
include(IncludeWhatYouUse)
AddIncludeWhatYouUse(${RUNNABLE_TARGET})

# copy config.json
add_custom_command(
        TARGET ${RUNNABLE_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/config.json
        ${CMAKE_CURRENT_BINARY_DIR}/config.json
)
