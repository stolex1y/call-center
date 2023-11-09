set(OBJ_LIB_TARGET callcenter)
set(STATIC_LIB_TARGET "${TARGET}_static")
set(TEST_TARGET "${TARGET}_test")

add_executable(${TEST_TARGET}
        call_center_service_test.cc
)
target_link_libraries(${TEST_TARGET} PRIVATE ${STATIC_LIB_TARGET})
AddTests(${TEST_TARGET})
#EnableCoverage(${OBJ_LIB_TARGET})