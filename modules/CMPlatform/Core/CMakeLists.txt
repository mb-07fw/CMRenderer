# CMPlatform_Core CMakeLists.txt

set(PARENT_DIR "${CMAKE_SOURCE_DIR}/modules/CMPlatform")
set(PARENT_BINARY_DIR "${CMAKE_BINARY_DIR}/modules/CMPlatform")
set(ROOT_DIR "${PARENT_DIR}/Core")

set(INCLUDE_DIR "${ROOT_DIR}/include")
set(SRC_DIR "${ROOT_DIR}/src")

set(PLATFORM_FILES
    "${INCLUDE_DIR}/Core_Export.hpp"
    "${INCLUDE_DIR}/IPlatform.hpp"
    "${INCLUDE_DIR}/Types.hpp"

    "${SRC_DIR}/IPlatform.cpp"
    "${SRC_DIR}/IWindow.cpp"
    "${SRC_DIR}/IGraphics.cpp"
    "${SRC_DIR}/Types.cpp"
)

add_library(CMPlatform_Core SHARED ${PLATFORM_FILES})

# Depend on CMShaders target, and inherit CMShaders defines.
add_dependencies(CMPlatform_Core CMShaders)
target_link_libraries(CMPlatform_Core PUBLIC CMShadersConfig)

# TODO: Have all sub-modules link to the same spdlog binary.
target_link_libraries(CMPlatform_Core PUBLIC spdlog::spdlog)

target_include_directories(CMPlatform_Core
    PUBLIC "${INCLUDE_DIR}"
    PUBLIC "${CMAKE_SOURCE_DIR}/vendor/submodules/spdlog/include"
    PRIVATE "${SRC_DIR}"
)

target_compile_definitions(CMPlatform_Core PRIVATE CM_PLATFORM_BUILD)

set(OUTPUT_DIR "${PARENT_BINARY_DIR}/Core/out")
SetTargetCommon(CMPlatform_Core ${OUTPUT_DIR})