# Root CMakeLists.txt

# To regenerate CMake caches (from root directory)
# cmake -S <root_directory> -B <build_directory> -G "Ninja"

# Build :
# cmake --build <out_directory> --config <configuration> [optional](--target <name>)

# For diagnostic output using MSBuild:
# cmake --build . --config Debug -- /v:diag

if (NOT WIN32)
    message(FATAL_ERROR "This project currently requires Windows to build.")
endif()

message(STATUS "[Root - Source Directory]: ${CMAKE_SOURCE_DIR}")
message(STATUS "[Root - Binary Directory]: ${CMAKE_BINARY_DIR}")

cmake_minimum_required(VERSION 3.31)
project(CMEngine)

# Assumes CMAKE_VERSION VERSION_GREATER 3.12
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT CMEngine)

# Tell CMake to reflect the actual folder structure of the
#   specified files in the Visual Studio Solution Explorer.
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADERS} ${SOURCES})

set (WARNINGS "")

if (MSVC)
    # /W4: Enable high warning level
    # /sdl: Security checks
    # /permissive-: Strict C++ compliance
    set(WARNINGS /W4 /sdl /permissive- /utf-8)

    # Force CMake to define these configurations in the generated solution by default.
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Dist" CACHE STRING "" FORCE)
else()
    set(WARNINGS -Wall)
endif()

function(SetTargetCommonWarnings target)
    target_compile_options(${target} PRIVATE ${WARNINGS})
endfunction()

function(SetTargetBuildDirectories target)
    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${target}/out
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${target}/out
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${target}/int
    )
endfunction()

add_subdirectory("CMCommon")
add_subdirectory("vendor/CMDep_DearImGui")
add_subdirectory("vendor/CMDep_yaml-cpp")

add_subdirectory("CMEngine")