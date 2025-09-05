# CMPlatform_WinImpl CMakeLists.txt
set(ROOT_DIR "${CMAKE_SOURCE_DIR}/modules/CMPlatform/WinImpl")
set(INCLUDE_DIR "${ROOT_DIR}/include")
set(SRC_DIR "${ROOT_DIR}/src")

set(PLATFORM_FILES
    "${INCLUDE_DIR}/WinImpl_Platform.hpp"
    "${INCLUDE_DIR}/WinImpl_WindowsTypes.hpp"
    "${INCLUDE_DIR}/WinImpl_WindowsDefines.hpp"
    "${INCLUDE_DIR}/WinImpl_Export.hpp"

    "${SRC_DIR}/WinImpl_Platform.cpp"
)

add_library(CMPlatform_WinImpl SHARED ${PLATFORM_FILES})

target_link_libraries(CMPlatform_WinImpl
    PUBLIC CMPlatform_Core
    PRIVATE CMImGui
    PRIVATE D3D11
    PRIVATE d3dcompiler
    PRIVATE DXGI
    PRIVATE dxguid
)

# get_target_property(defs CMShadersConfig INTERFACE_COMPILE_DEFINITIONS)
# message(STATUS "CMShadersConfig defs = ${defs}")

set(SHADER_DIR "${CMAKE_SOURCE_DIR}/CMEngine/resources/shaders/")
set(COMPILED_SHADER_DIR "${CMAKE_SOURCE_DIR}/CMEngine/resources/shaders/")

target_include_directories(CMPlatform_WinImpl PRIVATE ${INCLUDE_DIR} ${SRC_DIR})

# target_precompile_headers(CMPlatform_WinImpl PRIVATE "${ROOT_DIR}/src/CMPlatform/WinImpl_PCH.hpp")

set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/modules/CMPlatform/WinImpl/out")
SetTargetCommon(CMPlatform_WinImpl ${OUTPUT_DIR})

target_compile_definitions(CMPlatform_WinImpl
    PUBLIC CM_PLATFORM_IMPL_WIN
    PRIVATE CM_PLATFORM_IMPL_BUILD
)