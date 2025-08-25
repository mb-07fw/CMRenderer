# Root CMakeLists.txt

# To regenerate CMake caches (from root directory)
# cmake -S <root_directory> -B <build_directory> -G "Ninja"

# Build :
# cmake --build <out_directory> --config <configuration> [optional](--target <name>)

# For diagnostic output using MSBuild:
# cmake --build . --config Debug -- /v:diag

message(STATUS "Source Directory: ${CMAKE_SOURCE_DIR}")
message(STATUS "Binary Directory: ${CMAKE_BINARY_DIR}")

cmake_minimum_required(VERSION 3.31)
project(CMEngine)

# Assumes CMAKE_VERSION VERSION_GREATER 3.12
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT CMEngine)

set (WARNINGS "")

if (MSVC)
    # /W4: Enable high warning level
    # /sdl: Security checks
    # /permissive-: Strict C++ compliance
    set(WARNINGS /W4 /sdl /permissive- /utf-8)
else()
    set(WARNINGS -Wall)
endif()

function(SetTargetCommonWarnings target)
    target_compile_options(${target} PUBLIC ${WARNINGS})
endfunction()

add_subdirectory("vendor/submodules/spdlog")

add_subdirectory("modules/CMPlatform/Core")
add_subdirectory("modules/CMImGui")

add_subdirectory("CMEngine")
add_subdirectory("CMSandbox")