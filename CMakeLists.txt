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

function(SetTargetProperties target target_output_dir)
   set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${target_output_dir}
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${target_output_dir}
        LIBRARY_OUTPUT_DIRECTORY_DEBUG ${target_output_dir}

        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${target_output_dir}
        ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${target_output_dir}
        LIBRARY_OUTPUT_DIRECTORY_RELEASE ${target_output_dir}

        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${target_output_dir}
        ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${target_output_dir}
        LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${target_output_dir}

        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${target_output_dir}
        ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${target_output_dir}
        LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${target_output_dir}
    )
endfunction()

function (SetTargetCommon target target_output_dir)
    target_compile_options(${target} PUBLIC ${WARNINGS})

    target_compile_definitions(${target}
        PUBLIC UNICODE
        PUBLIC _UNICODE
        PUBLIC $<$<CONFIG:Debug>:CM_DEBUG=1>
        PUBLIC $<$<CONFIG:RelWithDebInfo>:CM_REL_DEBUG=1>
        PUBLIC $<$<CONFIG:Release>:CM_RELEASE=1>
        PUBLIC $<$<CONFIG:MinSizeRel>:CM_RELEASE_MIN=1>
    )

    SetTargetProperties(${target} ${target_output_dir})
endfunction()

add_subdirectory("vendor/submodules/spdlog")
add_subdirectory("vendor/submodules/DirectXTK")

add_subdirectory("modules/CMImGui")
add_subdirectory("modules/CMShaders")

add_subdirectory("CMEngine")

add_subdirectory("CMSandbox")