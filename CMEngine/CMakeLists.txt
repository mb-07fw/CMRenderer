# CMEngine CMakeLists.txt
set(ROOT_DIR "${CMAKE_SOURCE_DIR}/CMEngine")

set(ENGINE_FILES
    "include/Engine.hpp"
    "src/Export.hpp"
    "src/Types.hpp"

    "src/Macros.hpp"
    "src/Engine.cpp"
    "src/Types.cpp"
    "src/Platform.hpp"

    "src/PCH.hpp"
    "src/PCH_Main.hpp"
    "src/PCH.cpp"

    "src/Platform/Core/IGraphics.hpp"
    "src/Platform/Core/IPlatform.hpp"
    "src/Platform/Core/IWindow.hpp"
    "src/Platform/Core/IGraphics.cpp"
    "src/Platform/Core/IPlatform.cpp"
    "src/Platform/Core/IWindow.cpp"
)

if (WIN32)
    set(WINIMPL_PCH "${ROOT_DIR}/src/Platform/WinImpl/PCH_WinImpl.hpp")

    list(APPEND ENGINE_FILES
        "src/Platform/WinImpl/Graphics_WinImpl.hpp"
        "src/Platform/WinImpl/PCH_WinImpl.hpp"
        "src/Platform/WinImpl/PlatformOS_WinImpl.hpp"
        "src/Platform/WinImpl/PlatformOSFwd_WinImpl.hpp"
        "src/Platform/WinImpl/Platform_WinImpl.hpp"
        "src/Platform/WinImpl/ShaderLibrary_WinImpl.hpp"
        "src/Platform/WinImpl/Window_WinImpl.hpp"
        "src/Platform/WinImpl/Types_WinImpl.hpp"
        "src/Platform/WinImpl/GPUBuffer_WinImpl.hpp"
        "src/Platform/WinImpl/CPUBuffer_WinImpl.hpp"

        "src/Platform/WinImpl/Graphics_WinImpl.cpp"
        "src/Platform/WinImpl/Platform_WinImpl.cpp"
        "src/Platform/WinImpl/ShaderLibrary_WinImpl.cpp"
        "src/Platform/WinImpl/Window_WinImpl.cpp"
        "src/Platform/WinImpl/GPUBuffer_WinImpl.cpp"
        "src/Platform/WinImpl/CPUBuffer_WinImpl.cpp"
    )
endif()

add_library(CMEngine SHARED ${ENGINE_FILES})

if (WIN32)
    target_compile_definitions(CMEngine PUBLIC CM_ENGINE_PLATFORM_WINIMPL)
    target_precompile_headers(CMEngine PRIVATE ${WINIMPL_PCH})

    target_link_libraries(CMEngine
        PRIVATE D3D11
        PRIVATE d3dcompiler
        PRIVATE DXGI
        PRIVATE dxguid
        PRIVATE d2d1
        PRIVATE DWrite
        PRIVATE DirectXTK
    )
endif()

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

add_dependencies(CMEngine CMShaders)

target_link_libraries(CMEngine
    PUBLIC spdlog::spdlog
    PRIVATE CMImGui
    PRIVATE CMShadersConfig
)

# PRIVATE D3D11
# PRIVATE d3dcompiler
# PRIVATE DXGI
# PRIVATE dxguid