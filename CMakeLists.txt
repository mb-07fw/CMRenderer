# Root CMakeLists.txt

# To regenerate CMake caches (from root directory)
# cmake -S <root_directory> -B <build_directory> -G <your_generator_here>
# cmake -S . -B build -G "Visual Studio 17 2022" /m

# Build :
# cmake --build <out_directory> --config <configuration> [optional](--target <name>)

# For diagnostic output using MSBuild:
# cmake --build . --config Debug -- /v:diag

cmake_minimum_required(VERSION 3.31)
project(CMEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set (COMMON_WARNINGS "")

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

function (SetCommon target target_output_dir)
    target_compile_options(${target} PUBLIC ${COMMON_WARNINGS})
    target_compile_features(${target} PUBLIC cxx_std_20)

    target_compile_definitions(${target}
        PUBLIC UNICODE
        PUBLIC _UNICODE
        PRIVATE $<$<CONFIG:Debug>:CONFIG_DEBUG=1>
        PRIVATE $<$<CONFIG:RelWithDebInfo>:CONFIG_REL_DEBUG=1>
        PRIVATE $<$<NOT:$<CONFIG:Debug>>:CONFIG_RELEASE=1>
        PRIVATE $<$<CONFIG:MinSizeRel>:CONFIG_REL_MIN_SIZE=1 CONFIG_DIST=1>
    )
    
    SetTargetProperties(${target} ${target_output_dir})
endfunction()

add_subdirectory(Common)
add_subdirectory(Platform)
add_subdirectory(Engine)
add_subdirectory(Editor)