macro(find_package_required PACKAGE debpackage)
    find_package(${PACKAGE})
    string(TOUPPER "${PACKAGE}" PACKAGE_UPPERCASE)

    if(NOT ${PACKAGE_UPPERCASE}_FOUND AND NOT ${PACKAGE}_FOUND)
        message(FATAL_ERROR
                "Cmake module for ${PACKAGE} not found.\n"
                "Please install '${debpackage}' package(s).")
    endif()
endmacro()
