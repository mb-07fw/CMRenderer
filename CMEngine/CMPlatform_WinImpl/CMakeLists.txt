# CMPlatform_WinImpl CMakeLists.txt
set(ROOT_DIR "${CMAKE_SOURCE_DIR}/CMEngine/CMPlatform_WinImpl")

set(PLATFORM_FILES
    "${ROOT_DIR}/include/Platform/WinImpl_Platform.hpp"
    "${ROOT_DIR}/include/Platform/WinImpl_Window.hpp"
    "${ROOT_DIR}/include/Platform/WinImpl_Graphics.hpp"

    "${ROOT_DIR}/src/Platform/WinImpl_PCH.hpp"
    "${ROOT_DIR}/src/Platform/WinImpl_PCH.cpp"
    "${ROOT_DIR}/src/Platform/WinImpl_Platform.cpp"
    "${ROOT_DIR}/src/Platform/WinImpl_Window.cpp"
    "${ROOT_DIR}/src/Platform/WinImpl_Graphics.cpp"
)

add_library(CMPlatform_WinImpl SHARED ${PLATFORM_FILES})

find_file(D3D11_SDKLAYERS_FILE
    NAMES d3d11sdklayers.dll
    PATHS
        "${CMAKE_SYSTEM_ROOT}/System32"
        "C:/Windows/System32"
        "$ENV{SystemRoot}/System32"
)

if(NOT D3D11_SDKLAYERS_FILE)
    message(WARNING "CMPlatform_WinImpl: d3d11sdklayers.dll not found. Debug layer will not be available at runtime.")
else()
    message(STATUS "CMPlatform_WinImpl: Found: ${D3D11_SDKLAYERS_FILE}")

    target_compile_definitions(CMPlatform_WinImpl PRIVATE
        CM_PLATFORM_WIN_IMPL_D3D11_SDK_LAYERS_PATH="${D3D11_SDKLAYERS_FILE}"
        CM_PLATFORM_WIN_IMPL_D3D11_SDK_LAYERS_PATH_WIDE=L"${D3D11_SDKLAYERS_FILE}"
    )
endif()

target_link_libraries(CMPlatform_WinImpl PRIVATE
    CMPlatform_Core
    D3D11
    d3dcompiler
    DXGI
    dxguid
)

target_compile_definitions(CMPlatform_WinImpl PRIVATE CM_BUILD)
target_precompile_headers(CMPlatform_WinImpl PRIVATE "${ROOT_DIR}/src/Platform/WinImpl_PCH.hpp")

target_include_directories(CMPlatform_WinImpl PRIVATE
    "${ROOT_DIR}/include"
    "${CMAKE_SOURCE_DIR}/CMEngine/CMPlatform_Core/include"
    "${CMAKE_SOURCE_DIR}/CMEngine/CMPlatform_Core/src"
    "${CMAKE_SOURCE_DIR}/CMEngine/CMPlatform_WinImpl/include"
    "${CMAKE_SOURCE_DIR}/CMEngine/CMPlatform_WinImpl/src"
)

set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/build/$<CONFIG>/CMEngine/CMPlatform_WinImpl/out")

set_target_properties(CMPlatform_WinImpl PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
    ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}
)

SetTargetCommonWarnings(CMPlatform_WinImpl)