cmake_minimum_required(VERSION 3.14)
project(rppxml)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows specific settings
if(MSVC)
    add_compile_options(/utf-8)
    add_definitions(-D_UNICODE -DUNICODE)
endif()

# Add WDL as external project
include(FetchContent)
set(PROJECT_LIB_DIR ${PROJECT_SOURCE_DIR}/third_party)
FetchContent_Declare(
    wdl
    GIT_REPOSITORY https://github.com/justinfrankel/WDL
    SOURCE_DIR "${PROJECT_LIB_DIR}/WDL"
    GIT_TAG "origin/main"
)
FetchContent_MakeAvailable(wdl)

# Find required packages
find_package(OpenGL REQUIRED)

# WDL include directories
set(WDL_INCLUDE_DIRS
    ${PROJECT_LIB_DIR}/WDL
    ${PROJECT_LIB_DIR}/WDL/WDL
)

# WDL source files needed for ProjectContext
set(WDL_SOURCES
    ${PROJECT_LIB_DIR}/WDL/WDL/projectcontext.cpp
    ${PROJECT_LIB_DIR}/WDL/WDL/win32_utf8.c
)

# Option to control building test parser
option(BUILD_TEST_PARSER "Build the test parser executable" ON)

if(BUILD_TEST_PARSER)
    # Test parser executable
    add_executable(test_parse
        src/test_parse.cpp
        ${WDL_SOURCES}
    )

    target_include_directories(test_parse PRIVATE ${WDL_INCLUDE_DIRS})

    # Platform specific settings for test parser
    if(WIN32)
        target_link_libraries(test_parse PRIVATE
            wsock32
            ws2_32
            winmm
        )
    elseif(UNIX AND NOT APPLE)
        target_link_libraries(test_parse PRIVATE
            dl
            pthread
            rt
        )
    elseif(APPLE)
        find_library(COCOA_LIBRARY Cocoa)
        find_library(CARBON_LIBRARY Carbon)
        find_library(AUDIOUNIT_LIBRARY AudioUnit)
        find_library(COREAUDIO_LIBRARY CoreAudio)
        find_library(COREMIDI_LIBRARY CoreMIDI)
        
        target_link_libraries(test_parse PRIVATE
            ${COCOA_LIBRARY}
            ${CARBON_LIBRARY}
            ${AUDIOUNIT_LIBRARY}
            ${COREAUDIO_LIBRARY}
            ${COREMIDI_LIBRARY}
        )
    endif()
endif()

# Main project executable (to be added later)
# add_executable(${PROJECT_NAME}
#     src/your_main_project_files.cpp
# )
# target_include_directories(${PROJECT_NAME} PRIVATE ...)
# target_link_libraries(${PROJECT_NAME} PRIVATE ...)

# Installation
if(BUILD_TEST_PARSER)
    install(TARGETS test_parse
        RUNTIME DESTINATION bin
    )
endif()

# When you add the main project target, add its installation rules here
# install(TARGETS ${PROJECT_NAME}
#     RUNTIME DESTINATION bin
# )