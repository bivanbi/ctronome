cmake_minimum_required(VERSION 3.19)  # Specify the minimum CMake version
project(ctronome C) # Specify the project name and the language

# Add source files
set(SOURCES
        src/ctronome.c
        src/defaults.c
        src/dsp.c
        src/routines.c
)

include_directories(
        src
        cmake/generated_headers
)

# Add the executable target
add_executable(ctronome ${SOURCES})

set_target_properties(ctronome PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Git versioning
find_package(Git)

add_custom_target(git_version
        ${CMAKE_COMMAND} -D SRC=${CMAKE_SOURCE_DIR}/cmake/git_version.h.in
        -D DST=${CMAKE_SOURCE_DIR}/cmake/generated_headers/git_version.h
        -D GIT_EXECUTABLE=${GIT_EXECUTABLE}
        -P ${CMAKE_SOURCE_DIR}/cmake/generate_git_version_header.cmake
)

add_dependencies(ctronome git_version)
# end of Git versioning
