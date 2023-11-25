set(OBJ_LIB_TARGET callcenter)
set(STATIC_LIB_TARGET "${OBJ_LIB_TARGET}_static")
set(TEST_TARGET "${OBJ_LIB_TARGET}_test")

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
        fake/clock_interface.h
        task_manager_impl_test.cc
        utils.h
        utils.cc
        operator_set_test.cc
        fake/fake_operator.cc
        fake/fake_operator.h
)

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