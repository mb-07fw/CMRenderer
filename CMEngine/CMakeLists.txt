# CMEngine CMakeLists.txt

# Try to find fxc in PATH
find_program(FXC_EXECUTABLE fxc DOC "Path to the FX Compiler (fxc)")

# Try to search C: drive instead.
# (Currently only Program Files (x86) where Visual Studio 17 2022 installs it)
if (NOT FXC_EXECUTABLE)
    # Search for "fxc.exe" in any subdirectory under Program Files (x86)
    file(GLOB_RECURSE FXC_CANDIDATES "C:/Program Files (x86)/*fxc.exe")

    list(LENGTH FXC_CANDIDATES NUM_FXC_CANDIDATES)

    if (NUM_FXC_CANDIDATES GREATER 0)
        # Set FXC_EXECUTABLE to first FXC_CANDIDATE.
        list(GET FXC_CANDIDATES 0 FXC_EXECUTABLE)
    else()
        message(FATAL_ERROR 
            "FXC (Effect-Compiler Tool) not found in PATH or in Program Files (x86). "
            "Please install it or add it to your PATH environment variable."
        )
    endif()
else()
    message(STATUS "Found FXC: ${FXC_EXECUTABLE}")
endif()

set(ROOT_DIR "${CMAKE_SOURCE_DIR}/CMEngine")

# Core source files
file(GLOB_RECURSE CORE_SRC
	"${ROOT_DIR}/src/Core/CME_*.cpp"
	"${ROOT_DIR}/src/Core/CME_*.hpp"
)

# Core include files
file(GLOB_RECURSE CORE_INC "${ROOT_DIR}/include/Core/CME_*.hpp")

# Windows source files
file(GLOB_RECURSE WINDOWS_SRC
    "${ROOT_DIR}/src/Windows/CME_WN*.cpp"
    "${ROOT_DIR}/src/Windows/CME_WN*.hpp"
)

# Windows include files
file(GLOB_RECURSE WINDOWS_INC "${ROOT_DIR}/include/Windows/CME_WN*.h")

# DirectX source files
file(GLOB_RECURSE DX_SRC
    "${ROOT_DIR}/src/DirectX/CME_DX*.cpp"
    "${ROOT_DIR}/src/DirectX/CME_DX*.hpp"
)

# DirectX include files
file(GLOB_RECURSE DX_INC "${ROOT_DIR}/include/DirectX/CME_DX*.h")

# Vertex shaders
file(GLOB_RECURSE VERTEX_SHADERS "${ROOT_DIR}/resources/shaders/*VS.hlsl")

# Pixel shaders
file(GLOB_RECURSE PIXEL_SHADERS "${ROOT_DIR}/resources/shaders/*PS.hlsl")

source_group("CMEngine/Include Files/Core" FILES ${CORE_INC})
source_group("CMEngine/Source Files/Core" FILES ${CORE_SRC})

source_group("CMEngine/Include Files/Windows" FILES ${WINDOWS_INC})
source_group("CMEngine/Source Files/Windows" FILES ${WINDOWS_SRC})

source_group("CMEngine/Include Files/DirectX" FILES ${DX_INC})
source_group("CMEngine/Source Files/DirectX" FILES ${DX_SRC})

source_group("CMEngine/Shaders/Vertex" FILES ${VERTEX_SHADERS})
source_group("CMEngine/Shaders/Pixel" FILES ${PIXEL_SHADERS})

set(SHADER_FILES ${VERTEX_SHADERS} ${PIXEL_SHADERS})

set_source_files_properties(${VERTEX_SHADERS} PROPERTIES ShaderType "vs")
set_source_files_properties(${PIXEL_SHADERS} PROPERTIES ShaderType "ps")
set_source_files_properties(${SHADER_FILES} PROPERTIES ShaderModel "4_0")

set(SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/CMEngine/out")
file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})  # Ensure directory exists at configure time

# Prepare a list of compiled shader outputs
set(COMPILED_SHADERS)

foreach(FILE ${SHADER_FILES})
    get_filename_component(FILE_WE ${FILE} NAME_WE)
    get_source_file_property(shadertype ${FILE} ShaderType)
    get_source_file_property(shadermodel ${FILE} ShaderModel)

    set(CSO_OUTPUT "${SHADER_OUTPUT_DIR}/${FILE_WE}.cso")
    set(PDB_OUTPUT "${SHADER_OUTPUT_DIR}/${FILE_WE}.pdb")

    add_custom_command(
        OUTPUT ${CSO_OUTPUT} ${PDB_OUTPUT}
        COMMAND ${FXC_EXECUTABLE}
                /nologo /Emain
                /T${shadertype}_${shadermodel}
                $<IF:$<CONFIG:DEBUG>,/Od,/O1>
                /Zi
                /Fo ${CSO_OUTPUT}
                /Fd ${PDB_OUTPUT}
                ${FILE}
        MAIN_DEPENDENCY ${FILE}
        COMMENT "Compiling HLSL shader: ${FILE}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        VERBATIM
    )

    list(APPEND COMPILED_SHADERS ${CSO_OUTPUT})
endforeach()

# Create a buildable target from shader outputs
add_custom_target(shaders ALL DEPENDS ${COMPILED_SHADERS})

add_executable(CMEngine 
    ${CORE_SRC}
    ${CORE_INC}
    ${WINDOWS_SRC}
    ${WINDOWS_INC}
    ${DX_SRC}
    ${DX_INC}
    ${PIXEL_SHADERS}
    ${VERTEX_SHADERS}
)

add_dependencies(CMEngine shaders)

target_precompile_headers(CMEngine PRIVATE "include/Core/CME_PCH.hpp")

target_compile_definitions(CMEngine PRIVATE
    $<$<CONFIG:Debug>:CM_DEBUG>
    $<$<CONFIG:Release>:CM_NDEBUG CM_RELEASE>
    $<$<CONFIG:Dist>:CM_NDEBUG CM_DIST>
)

target_compile_definitions(CMEngine PRIVATE UNICODE _UNICODE CM_CONFIG="$<CONFIG>" CM_CMAKE)

target_include_directories(CMEngine PUBLIC
    "${CMAKE_SOURCE_DIR}/CMEngine/include"
    "${CMAKE_SOURCE_DIR}/CMCommon/include"
    "${CMAKE_SOURCE_DIR}/CMRenderer/include"
    "${CMAKE_SOURCE_DIR}/vendor/CMDep_DearImGui"
    "${CMAKE_SOURCE_DIR}/vendor/CMDep_yaml-cpp/include"
)

SetTargetCommonWarnings(CMEngine)
SetTargetBuildDirectories(CMEngine)

# TODO: Figure out DirectXTK's CMake integration. Turn the current CMDep's into submodules.

target_link_libraries(CMEngine PRIVATE
    CMCommon
    CMDep_DearImGui
    CMDep_yaml-cpp
)