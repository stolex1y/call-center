set(OBJ_LIB_TARGET ${CMAKE_PROJECT_NAME})
set(STATIC_LIB_TARGET "${OBJ_LIB_TARGET}-static")
set(TEST_TARGET "${OBJ_LIB_TARGET}-test")

add_executable(${TEST_TARGET}
        call_center_test.cc
        configuration_adapter.cc
        configuration_adapter.h
        fake/fake_clock.cc
        fake/fake_clock.h
        fake/fake_task_manager.cc
        fake/fake_task_manager.h
        fake/fake_call_detailed_record.cc
        fake/fake_call_detailed_record.h
        core/tasks/task_manager_impl_test.cc
        utils.h
        utils.cc
        operator_set_test.cc
        mock/mock_operator.cc
        mock/mock_operator.h
        core/queueing_system/metrics/queueing_system_metrics_test.cc
        fake/fake_service_loader.cc
        fake/fake_service_loader.h
)
target_include_directories(${TEST_TARGET} PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")

# clang-format
include(Format)
Format(${TEST_TARGET} .)

#clang-tidy
#include(ClangTidy)
#AddClangTidy(${TEST_TARGET})

#iwyu
include(IncludeWhatYouUse)
AddIncludeWhatYouUse(${TEST_TARGET})

target_link_libraries(${TEST_TARGET} PRIVATE ${STATIC_LIB_TARGET})

AddTests(${TEST_TARGET})
#EnableCoverage(${OBJ_LIB_TARGET})