cmake_minimum_required(VERSION 3.19)  # Specify the minimum CMake version
project(ctronome C) # Specify the project name and the language

file(GLOB SOURCES "src/*.c")

include_directories(
        src
        cmake/generated_headers
)

# Git versioning
find_package(Git)

add_custom_target(git_version
        ${CMAKE_COMMAND} -D SRC=${CMAKE_SOURCE_DIR}/cmake/git_version.h.in
        -D DST=${CMAKE_SOURCE_DIR}/cmake/generated_headers/git_version.h
        -D GIT_EXECUTABLE=${GIT_EXECUTABLE}
        -P ${CMAKE_SOURCE_DIR}/cmake/generate_git_version_header.cmake
)
# end of Git versioning

# Discover supported audio systems
include(CheckIncludeFile)
check_include_file(linux/soundcard.h HAVE_SOUNDCARD_H)
if (HAVE_SOUNDCARD_H)
    add_definitions(-DHAVE_SOUNDCARD_H)
    list(APPEND SOURCES src/audio_adapters/dsp_adapter.c)
endif ()

check_include_file(pulse/simple.h HAVE_PULSEAUDIO_SIMPLE_H)
if (HAVE_PULSEAUDIO_SIMPLE_H)
    add_definitions(-DHAVE_PULSEAUDIO_SIMPLE_H)
    list(APPEND SOURCES src/audio_adapters/pulseaudio_adapter.c)

    link_libraries(pulse-simple pulse)
endif ()

# End of Discover supported audio systems

# Install target
message(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
# For ctronome binary will know where to look for the assets
add_compile_definitions(ASSETS_INSTALL_DIR="${CMAKE_INSTALL_PREFIX}/share/ctronome/assets")

# Add the executable target
add_executable(ctronome ${SOURCES})

set_target_properties(ctronome PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

add_dependencies(ctronome git_version)

# Install the executable
install(TARGETS ctronome
        RUNTIME DESTINATION bin
)

install(DIRECTORY assets
        DESTINATION share/ctronome)
# end of Install target
