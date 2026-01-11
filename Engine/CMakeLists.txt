# Engine CMakeLists.txt

set(Src_Files
    "src/Engine.cpp"
)

set(Inc_Files
    "include/Engine.hpp"
)

set(Build_Files ${Src_Files} ${Inc_Files})

add_library(Engine STATIC ${Build_Files})

target_link_libraries(Engine PUBLIC Platform)

target_include_directories(Engine PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_compile_definitions(Engine PUBLIC 
    ENGINE_RESOURCES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources/"
    ENGINE_RESOURCES_DIRW=L"${CMAKE_CURRENT_SOURCE_DIR}/resources/"
    ENGINE_RESOURCES_SHADERS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders/"
    ENGINE_RESOURCES_SHADERS_DIRW="${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders/"
)

set(Output_Dir "${CMAKE_BINARY_DIR}/Engine/out")

SetCommon(Engine ${Output_Dir})