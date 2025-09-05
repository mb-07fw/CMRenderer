# CMEngine CMakeLists.txt
set(ROOT_DIR "${CMAKE_SOURCE_DIR}/CMEngine")

set(ENGINE_FILES
    "${ROOT_DIR}/include/Engine.hpp"
    "${ROOT_DIR}/src/Engine_Export.hpp"
    "${ROOT_DIR}/src/Engine.cpp"
    "${ROOT_DIR}/src/Macros.hpp"
)

add_library(CMEngine SHARED ${ENGINE_FILES})

target_include_directories(CMEngine PUBLIC
    "${CMAKE_SOURCE_DIR}/CMEngine/include"
    "${CMAKE_SOURCE_DIR}/CMEngine/src"
)

target_compile_definitions(CMEngine PRIVATE 
    CM_ENGINE_CONFIG_STR="$<CONFIG>"
    CM_ENGINE_SOURCE_DIR_STR="${CMAKE_SOURCE_DIR}"
    CM_ENGINE_BUILD
)

set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/CMEngine/out")
SetTargetCommon(CMEngine ${OUTPUT_DIR})

target_link_libraries(CMEngine PRIVATE CMPlatform)

get_target_property(platform_include_dirs CMPlatform INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(platform_core_include_dirs CMPlatform_Core INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(platform_defines CMPlatform INTERFACE_COMPILE_DEFINITIONS)

message(STATUS "Platform defines: ${platform_defines}")

target_include_directories(CMEngine PUBLIC ${platform_include_dirs})
target_include_directories(CMEngine PUBLIC ${platform_core_include_dirs})