# Platform CMakeLists.txt

set(Src_Files
    "include/IRefCounted.hpp"

    "src/IApi.hpp" 
    "src/IApiFactory.hpp"
    "src/IApiResources.cpp"
    "src/IApiResources.hpp"

    "src/IContext.hpp"

    "src/ILogger.hpp"

    "src/ImplRefCounted.hpp"
    "src/ImplRefCountedUndef.hpp"

    "src/IPlatform.cpp"
    "src/IPlatform.hpp"

    "src/IWindow.hpp"

    "src/PlatformFailure.hpp"

    "src/Types.cpp"
    "src/Types.hpp"

    "src/Backend/RefCounted.hpp"

    "src/Backend/Win32/Platform_Win32.cpp"
    "src/Backend/Win32/Platform_Win32.hpp"

    "src/Backend/Win32/Types_Win32.hpp"

    "src/Backend/Win32/Window_Win32.cpp"
    "src/Backend/Win32/Window_Win32.hpp"

    "src/Backend/Win32/Resources_Win32.cpp"
    "src/Backend/Win32/Resources_Win32.hpp"

    "src/Backend/Win32/Context_Win32.cpp"
    "src/Backend/Win32/Context_Win32.hpp"
    
    "src/Backend/Win32/D3D/_11/ApiFactory_D3D11.cpp"
    "src/Backend/Win32/D3D/_11/ApiFactory_D3D11.hpp"

    "src/Backend/Win32/D3D/_11/ApiResources_D3D11.cpp"
    "src/Backend/Win32/D3D/_11/ApiResources_D3D11.hpp"

    "src/Backend/Win32/D3D/_11/Api_D3D11.cpp"
    "src/Backend/Win32/D3D/_11/Api_D3D11.hpp"

    "src/Backend/Win32/PlatformOS_Win32.hpp"
    "src/Backend/Win32/PlatformFwd_Win32.hpp"
)

set(Inc_Files "include/Platform.hpp")

set(Build_Files ${Src_Files} ${Inc_Files})

add_library(Platform SHARED ${Build_Files})

target_include_directories(Platform PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_compile_definitions(Platform
    PRIVATE PLATFORM_BUILD=1
)

target_link_libraries(Platform PUBLIC Common)

if (WIN32)
    target_link_libraries(Platform PRIVATE
        d3dcompiler
        d3d11
        dxguid
    )
endif()

target_precompile_headers(Platform PRIVATE "src/PCH.hpp")

set(Output_Dir "${CMAKE_BINARY_DIR}/Platform/out")

SetCommon(Platform ${Output_Dir})