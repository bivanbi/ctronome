if(GIT_EXECUTABLE)
    get_filename_component(SRC_DIR ${SRC} DIRECTORY)
    # Generate a git-describe version string from Git repository tags
    execute_process(
            COMMAND ${GIT_EXECUTABLE} describe --abbrev=4 --dirty --always --tags
            WORKING_DIRECTORY ${SRC_DIR}
            OUTPUT_VARIABLE GIT_OUTPUT_VERSION
            RESULT_VARIABLE GIT_ERROR_CODE
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (GIT_ERROR_CODE)
        message(WARNING "Git version get (describe) failed with error code ${GIT_ERROR_CODE}")
        set(GIT_VERSION v0.0.0-unknown)
    else()
        set(GIT_VERSION ${GIT_OUTPUT_VERSION})
    endif()
else()
    message(WARNING "Git executable not found. Version information will not be available.")
    set(GIT_VERSION v0.0.0-unknown)
endif()

configure_file(${SRC} ${DST} @ONLY)