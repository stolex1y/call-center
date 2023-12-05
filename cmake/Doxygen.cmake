include_guard(GLOBAL)

macro(UseDoxygenAwesomeCss)
    include(FetchContent)
    FetchContent_Declare(doxygen-awesome-css
            GIT_REPOSITORY
            https://github.com/jothepro/doxygen-awesome-css.git
            GIT_TAG
            v1.6.0
    )
    FetchContent_MakeAvailable(doxygen-awesome-css)
    set(DOXYGEN_GENERATE_TREEVIEW YES)
    set(DOXYGEN_HAVE_DOT YES)
    set(DOXYGEN_DOT_IMAGE_FORMAT svg)
    set(DOXYGEN_DOT_TRANSPARENT YES)
    set(DOXYGEN_HTML_EXTRA_STYLESHEET
            ${doxygen-awesome-css_SOURCE_DIR}/doxygen-awesome.css)
endmacro()

function(Doxygen)
    find_package(Doxygen)
    if (NOT DOXYGEN_FOUND)
        add_custom_target(doxygen COMMAND false
                COMMENT "Doxygen not found")
        return()
    endif ()
    set(DOXYGEN_TARGET_NAME "doxygen")
    set(DOXYGEN_OUTPUT_LANGUAGE "Russian")
    set(DOXYGEN_PROJECT_NAME "Call center")
    set(DOXYGEN_PROJECT_NUMBER ${CMAKE_PROJECT_VERSION})
    set(DOXYGEN_RECURSIVE YES)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_HTML_OUTPUT
            ${PROJECT_BINARY_DIR}/${DOXYGEN_TARGET_NAME})
    UseDoxygenAwesomeCss()
    doxygen_add_docs(${DOXYGEN_TARGET_NAME}
            ${CMAKE_SOURCE_DIR}/docs
            src
            test
            COMMENT "Generate HTML documentation"
#            CONFIG_FILE ${CMAKE_SOURCE_DIR}/docs/doxygen.conf
    )
endfunction()
