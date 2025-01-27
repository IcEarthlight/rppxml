cmake_minimum_required(VERSION 3.11)
project(rppxml)

# set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# windows specific settings
if(MSVC)
    add_compile_options(/utf-8)
    add_definitions(-D_UNICODE -DUNICODE)
endif()

# set third-party libraries directory
set(PROJECT_LIB_DIR ${PROJECT_SOURCE_DIR}/third_party)

# add external dependencies
include(FetchContent)

# add WDL
FetchContent_Declare(
    wdl
    GIT_REPOSITORY https://github.com/justinfrankel/WDL
    SOURCE_DIR "${PROJECT_LIB_DIR}/WDL"
    GIT_TAG "origin/main"
)

# add pybind11
FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11
    SOURCE_DIR "${PROJECT_LIB_DIR}/pybind11"
    GIT_TAG "origin/master"
)

FetchContent_MakeAvailable(wdl pybind11)

# find required packages
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

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

# option to control building test parser
option(BUILD_TEST_PARSER "Build the test parser executable" ON)

if(BUILD_TEST_PARSER)
    # test parser executable
    add_executable(test_parse
        src/test_parse.cpp
        ${WDL_SOURCES}
    )

    target_include_directories(test_parse PRIVATE ${WDL_INCLUDE_DIRS})

    # platform specific settings for test parser
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

# python module
pybind11_add_module(rppxml 
    src/rppxml.cpp
    ${WDL_SOURCES}
)

target_include_directories(rppxml PRIVATE 
    ${WDL_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/src
)

if(WIN32)
    target_link_libraries(rppxml PRIVATE
        wsock32
        ws2_32
        winmm
    )
elseif(UNIX AND NOT APPLE)
    target_link_libraries(rppxml PRIVATE
        dl
        pthread
        rt
    )
elseif(APPLE)
    target_link_libraries(rppxml PRIVATE
        ${COCOA_LIBRARY}
        ${CARBON_LIBRARY}
        ${AUDIOUNIT_LIBRARY}
        ${COREAUDIO_LIBRARY}
        ${COREMIDI_LIBRARY}
    )
endif()

# installation
if(BUILD_TEST_PARSER)
    install(TARGETS test_parse
        RUNTIME DESTINATION bin
    )
endif()

install(TARGETS rppxml 
    LIBRARY DESTINATION ${Python3_SITEARCH}
)