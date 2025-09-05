# CMPlatform_WinImpl CMakeLists.txt
set(ROOT_DIR "${CMAKE_SOURCE_DIR}/modules/CMPlatform/WinImpl")
set(INCLUDE_DIR "${ROOT_DIR}/include")
set(SRC_DIR "${ROOT_DIR}/src")

set(PLATFORM_FILES
    "${INCLUDE_DIR}/CMPlatform/WinImpl_Platform.hpp"
    "${INCLUDE_DIR}/CMPlatform/WinImpl_Window.hpp"
    "${INCLUDE_DIR}/CMPlatform/WinImpl_Graphics.hpp"

    "${SRC_DIR}/CMPlatform/WinImpl_PCH.hpp"
    "${SRC_DIR}/CMPlatform/WinImpl_PCH.cpp"
    "${SRC_DIR}/CMPlatform/WinImpl_Platform.cpp"
    "${SRC_DIR}/CMPlatform/WinImpl_Window.cpp"
    "${SRC_DIR}/CMPlatform/WinImpl_Graphics.cpp"
    "${SRC_DIR}/CMPlatform/WinImpl_ShaderLibrary.cpp"
    "${SRC_DIR}/CMPlatform/WinImpl_ShaderLibrary.hpp"
)

add_library(CMPlatform_WinImpl SHARED ${PLATFORM_FILES})

target_link_libraries(CMPlatform_WinImpl
    PUBLIC CMPlatform_Core
    PRIVATE CMImGui
    PRIVATE D3D11
    PRIVATE d3dcompiler
    PRIVATE DXGI
    PRIVATE dxguid
    INTERFACE CMShaders
)

get_target_property(defs CMShadersConfig INTERFACE_COMPILE_DEFINITIONS)
message(STATUS "CMShadersConfig defs = ${defs}")

set(SHADER_DIR "${CMAKE_SOURCE_DIR}/CMEngine/resources/shaders/")
set(COMPILED_SHADER_DIR "${CMAKE_SOURCE_DIR}/CMEngine/resources/shaders/")

target_compile_definitions(CMPlatform_WinImpl PRIVATE CM_BUILD)
target_include_directories(CMPlatform_WinImpl PRIVATE ${INCLUDE_DIR} ${SRC_DIR})

target_precompile_headers(CMPlatform_WinImpl PRIVATE "${ROOT_DIR}/src/CMPlatform/WinImpl_PCH.hpp")

set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/modules/CMPlatform/WinImpl/out")

set_target_properties(CMPlatform_WinImpl PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
    ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}
)

SetTargetCommonWarnings(CMPlatform_WinImpl)