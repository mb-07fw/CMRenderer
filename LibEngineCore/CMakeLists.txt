# LibEngineCore CMakeLists.txt
set(ROOT_DIR "${CMAKE_SOURCE_DIR}/LibEngineCore")

set(ENGINE_FILES
    "include/EngineCore.hpp"

    "src/Macros.hpp"
    "src/Log.hpp"
    "src/Graphics.hpp"
    "src/Platform.hpp"
    "src/PlatformUtil.hpp"
    "src/Types.hpp"
    "src/Component.hpp"
    "src/Math.hpp"
    "src/BatchRenderer.hpp"
    "src/Renderer.hpp"
    "src/ECS/TypeID.hpp"
    "src/ECS/SparseSet.hpp"
    "src/ECS/Entity.hpp"
    "src/ECS/ECS.hpp"
    "src/Asset/Asset.hpp"
    "src/Asset/AssetID.hpp"
    "src/Asset/AssetManager.hpp"
    "src/Scene/Node.hpp"
    "src/Scene/CameraSystem.hpp"
    "src/Scene/Scene.hpp"
    "src/Scene/SceneManager.hpp"

    "src/EngineCore.cpp"
    "src/Types.cpp"
    "src/Component.cpp"
    "src/BatchRenderer.cpp"
    "src/Renderer.cpp"
    "src/ECS/TypeID.cpp"
    "src/ECS/Entity.cpp"
    "src/ECS/ECS.cpp"
    "src/Asset/AssetID.cpp"
    "src/Asset/AssetManager.cpp"
    "src/Scene/Node.cpp"
    "src/Scene/CameraSystem.cpp"
    "src/Scene/Scene.cpp"
    "src/Scene/SceneManager.cpp"

    "src/PCH.hpp"
    "src/PCH_Main.hpp"
    "src/PCH.cpp"

    "src/Platform/Core/IGraphics.hpp"
    "src/Platform/Core/IPlatform.hpp"
    "src/Platform/Core/IPlatformUUID.hpp"
    "src/Platform/Core/IPlatformUtil.hpp"
    "src/Platform/Core/IWindow.hpp"
    "src/Platform/Core/IUploadable.hpp"
    "src/Platform/Core/InputElement.hpp"
    "src/Platform/Core/InputLayout.hpp"
    "src/Platform/Core/Shader.hpp"

    "src/Platform/Core/IGraphics.cpp"
    "src/Platform/Core/IPlatform.cpp"
    "src/Platform/Core/IWindow.cpp"
)

if (WIN32)
    list(APPEND ENGINE_FILES
        "src/Platform/WinImpl/IDXUploadable_WinImpl.hpp"
        "src/Platform/WinImpl/Graphics_WinImpl.hpp"
        "src/Platform/WinImpl/PCH_WinImpl.hpp"
        "src/Platform/WinImpl/PlatformOS_WinImpl.hpp"
        "src/Platform/WinImpl/PlatformOSFwd_WinImpl.hpp"
        "src/Platform/WinImpl/Platform_WinImpl.hpp"
        "src/Platform/WinImpl/PlatformUtil_WinImpl.hpp"
        "src/Platform/WinImpl/ShaderRegistry_WinImpl.hpp"
        "src/Platform/WinImpl/Window_WinImpl.hpp"
        "src/Platform/WinImpl/Types_WinImpl.hpp"
        "src/Platform/WinImpl/GPUBuffer_WinImpl.hpp"
        "src/Platform/WinImpl/InputLayout_WinImpl.hpp"

        "src/Platform/WinImpl/Graphics_WinImpl.cpp"
        "src/Platform/WinImpl/Platform_WinImpl.cpp"
        "src/Platform/WinImpl/PlatformUtil_WinImpl.cpp"
        "src/Platform/WinImpl/ShaderRegistry_WinImpl.cpp"
        "src/Platform/WinImpl/Window_WinImpl.cpp"
        "src/Platform/WinImpl/GPUBuffer_WinImpl.cpp"
        "src/Platform/WinImpl/InputLayout_WinImpl.cpp"
    )
endif()

add_library(LibEngineCore STATIC ${ENGINE_FILES})

if (WIN32)
    target_compile_definitions(LibEngineCore PUBLIC ENGINE_CORE_PLATFORM_WINIMPL)

    target_link_libraries(LibEngineCore
        PRIVATE D3D11
        PRIVATE d3dcompiler
        PRIVATE DXGI
        PRIVATE dxguid
        PRIVATE d2d1
        PRIVATE DWrite
        PRIVATE DirectXTK
        # (CoCreateGuid -- combaseapi.h)
        PRIVATE Ole32
        # (UuidToString -- rpcdce.h)
        PRIVATE Rpcrt4.lib
    )
endif()

target_include_directories(LibEngineCore PUBLIC
    "${ROOT_DIR}/include"
    "${ROOT_DIR}/src"
)

target_compile_definitions(LibEngineCore PRIVATE 
    ENGINE_CORE_CONFIG="$<CONFIG>"
    ENGINE_CORE_SOURCE_DIRECTORY="${CMAKE_SOURCE_DIR}"
)

target_precompile_headers(LibEngineCore PRIVATE "src/PCH.hpp")

set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/LibEngineCore/out")
SetTargetCommon(LibEngineCore ${OUTPUT_DIR})

add_dependencies(LibEngineCore CMShaders)

target_link_libraries(LibEngineCore
    PUBLIC spdlog::spdlog
    PUBLIC assimp::assimp
    PRIVATE CMImGui
    PRIVATE CMShadersConfig
)