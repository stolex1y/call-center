install(TARGETS "${CMAKE_PROJECT_NAME}-runnable" RUNTIME COMPONENT runtime)

# CPack configuration
set(CPACK_PACKAGE_VENDOR "Alexey Filimonov")
set(CPACK_PACKAGE_CONTACT "filimonov.alks@gmail.com")
set(CPACK_PACKAGE_DESCRIPTION "Call center")
include(CPack)
