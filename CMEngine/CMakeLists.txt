# CMEngine CMakeLists.txt
set(ROOT_DIR "${CMAKE_SOURCE_DIR}/CMEngine")

set(ENGINE_FILES
    "${ROOT_DIR}/include/Engine.hpp"
    "${ROOT_DIR}/src/Export.hpp"
    "${ROOT_DIR}/src/Types.hpp"

    "${ROOT_DIR}/src/Macros.hpp"
    "${ROOT_DIR}/src/Engine.cpp"
    "${ROOT_DIR}/src/Types.cpp"
    "${ROOT_DIR}/src/Platform.hpp"

    "${ROOT_DIR}/src/PCH.hpp"
    "${ROOT_DIR}/src/PCH_Main.hpp"
    "${ROOT_DIR}/src/PCH.cpp"

    "${ROOT_DIR}/src/Platform/Core/IGraphics.hpp"
    "${ROOT_DIR}/src/Platform/Core/IPlatform.hpp"
    "${ROOT_DIR}/src/Platform/Core/IWindow.hpp"
    "${ROOT_DIR}/src/Platform/Core/IGraphics.cpp"
    "${ROOT_DIR}/src/Platform/Core/IPlatform.cpp"
    "${ROOT_DIR}/src/Platform/Core/IWindow.cpp"
)

if (WIN32)
    set(WINIMPL_PCH "${ROOT_DIR}/src/Platform/WinImpl/PCH_WinImpl.hpp")

    list(APPEND ENGINE_FILES
        "${ROOT_DIR}/src/Platform/WinImpl/Graphics_WinImpl.hpp"
        "${ROOT_DIR}/src/Platform/WinImpl/PCH_WinImpl.hpp"
        "${ROOT_DIR}/src/Platform/WinImpl/PlatformOS_WinImpl.hpp"
        "${ROOT_DIR}/src/Platform/WinImpl/PlatformOSFwd_WinImpl.hpp"
        "${ROOT_DIR}/src/Platform/WinImpl/Platform_WinImpl.hpp"
        "${ROOT_DIR}/src/Platform/WinImpl/ShaderLibrary_WinImpl.hpp"
        "${ROOT_DIR}/src/Platform/WinImpl/Window_WinImpl.hpp"
        "${ROOT_DIR}/src/Platform/WinImpl/Types_WinImpl.hpp"

        "${ROOT_DIR}/src/Platform/WinImpl/Graphics_WinImpl.cpp"
        "${ROOT_DIR}/src/Platform/WinImpl/Platform_WinImpl.cpp"
        "${ROOT_DIR}/src/Platform/WinImpl/ShaderLibrary_WinImpl.cpp"
        "${ROOT_DIR}/src/Platform/WinImpl/Window_WinImpl.cpp"
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