# Platform CMakeLists.txt

set(Src_Files 
    "src/IApi.hpp"

    "src/IContext.hpp"

    "src/ILogger.hpp"

    "src/IPlatform.hpp"
    "src/IPlatform.cpp"

    "src/IWindow.hpp"


    "src/Win32/Platform_Win32.hpp"
    "src/Win32/Platform_Win32.cpp"

    "src/Win32/Window_Win32.hpp"
    "src/Win32/Window_Win32.cpp"

    "src/Win32/Context_Win32.hpp"
    "src/Win32/Context_Win32.cpp"
    
    "src/Win32/D3D/_11/Api_D3D11.hpp"
    "src/Win32/D3D/_11/Api_D3D11.cpp"

    "src/Win32/PlatformOS_Win32.hpp"
    "src/Win32/PlatformFwd_Win32.hpp"
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