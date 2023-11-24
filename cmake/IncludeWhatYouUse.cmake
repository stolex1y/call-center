function(AddIncludeWhatYouUse target)
    find_program(IWYU_PATH iwyu REQUIRED)
    set_target_properties(${target}
            PROPERTIES CXX_INCLUDE_WHAT_YOU_USE
            "${IWYU_PATH};--error_always;--no_fwd_decls"
    )
endfunction()
