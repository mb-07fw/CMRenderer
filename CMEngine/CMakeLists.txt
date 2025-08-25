# CMEngine CMakeLists.txt
set(ROOT_DIR "${CMAKE_SOURCE_DIR}/CMEngine")

set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/build/$<CONFIG>/CMEngine/out")

set(ENGINE_FILES
    "${ROOT_DIR}/include/Engine.hpp"
    "${ROOT_DIR}/src/Engine.cpp"
    "${ROOT_DIR}/src/Macros.hpp"
)

add_library(CMEngine SHARED ${ENGINE_FILES})

target_include_directories(CMEngine PUBLIC
    "${CMAKE_SOURCE_DIR}/CMEngine/include"
    "${CMAKE_SOURCE_DIR}/CMEngine/src"
    "${CMAKE_SOURCE_DIR}/modules/CMPlatform/Core/include"
)

set_target_properties(CMEngine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
    ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}
)

target_compile_definitions(CMEngine PUBLIC
    $<$<CONFIG:Debug>:CM_DEBUG>
    $<$<CONFIG:RelWithDebInfo>:CM_NDEBUG CM_RELEASE>
    $<$<CONFIG:Release>:CM_NDEBUG CM_DIST>
    CM_BUILD
)

target_compile_definitions(CMEngine PRIVATE
    CM_CONFIG="$<CONFIG>"
    CM_CMAKE
    CM_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

SetTargetCommonWarnings(CMEngine)

target_link_libraries(CMEngine PRIVATE CMPlatform_Core CMImGui)

# Note for me in the future, user define is different from FXC_EXECUTABLE as CMake
#   remembers find_program through the cache, and was causing it to use the x86
#   version of FXC as it was always the first it found outside of Visual Studio's CMake.
# if (DEFINED FXC_EXECUTABLE_USER AND EXISTS "${FXC_EXECUTABLE_USER}")
#     message(STATUS "Using user-specified FXC: ${FXC_EXECUTABLE_USER}")
#     set(FXC_EXECUTABLE_USER "" CACHE FILEPATH "Optional user-specified path to FXC executable.")
#     set(FXC_EXECUTABLE ${FXC_EXECUTABLE_USER})
# else()
#     if(DEFINED FXC_EXECUTABLE_USER AND NOT EXISTS "${FXC_EXECUTABLE_USER}")
#         message(STATUS "FXC_EXECUTABLE_USER was defined, but doesn't exist.")
#     endif()
# 
#     # Note for anybody else struggling with finding FXC, you may needs to manually 
#     #   set the HINTS path to match your system's locatation of FXC.
#     find_program(FXC_EXECUTABLE fxc
#         DOC "Path to the FX Compiler (fxc)"
#     )
# 
#     if (FXC_EXECUTABLE)
#         message(STATUS "Found FXC in PATH: ${FXC_EXECUTABLE}")
#     else()
#         message(STATUS "Failed to find FXC in PATH.")
#     endif()
# 
#     if (FXC_EXECUTABLE MATCHES [[/x64/]])
#         message(STATUS "FXC is already x64: ${FXC_EXECUTABLE}")
#     else()
#         # Try to search C: drive instead.
#         # (Currently only Program Files (x86) where Visual Studio 17 2022 installs it)
# 
#         if (FXC_EXECUTABLE)
#             message(STATUS "Searching for FXC in Program Files (x86) again. (A x86 FXC was found instead of an x64)")
#         else()
#             message(STATUS "Searching for FXC in Program Files (x86).")
#         endif()
# 
#         # Extract the environment variable into a proper CMake variable for use in glob-ing.
#         set(PFX86 "$ENV{ProgramFiles\(x86\)}")
# 
#         message(STATUS "PF: ${PFX86}")
# 
#         # Search for "fxc.exe" in any subdirectory under Program Files (x86)\Windows Kits\10\bin\
#         file(GLOB_RECURSE FXC_CANDIDATES "${PFX86}/Windows Kits/10/bin/*/x64/fxc.exe")
# 
#         list(LENGTH FXC_CANDIDATES NUM_FXC_CANDIDATES)
# 
#         foreach(path IN LISTS FXC_CANDIDATES)
#             message(STATUS "Candidate FXC: ${path}")
#         endforeach()
# 
#         if (NUM_FXC_CANDIDATES GREATER 0)
#             foreach(path IN LISTS FXC_CANDIDATES)
#                 if("${path}" MATCHES [[/x64/]])
#                     set(FXC_EXECUTABLE "${path}")
#                     break()
#                 endif()
#             endforeach()
#         else()
#             message(FATAL_ERROR
#                 "FXC (Effect-Compiler Tool) not found in Program Files (x86). "
#                 "You may use `-DFXC_EXECUTABLE_USER=\"...\" as a supplimentary path to a custom x64 FXC install instead."
#             )
#         endif()
# 
#         message(STATUS "Final FXC Candidate: ${FXC_EXECUTABLE}")
#     endif()
# endif()

# set(CMENGINE_INC "${ROOT_DIR}/include/Engine.hpp")
# set(CMENGINE_SRC "${ROOT_DIR}/src/Engine.cpp")

# Common source files
# file(GLOB_RECURSE COMMON_SRC
# 	"${ROOT_DIR}/src/Common/*.cpp"
# 	"${ROOT_DIR}/src/Common/*.hpp"
# )

# Common include files
# file(GLOB_RECURSE COMMON_INC "${ROOT_DIR}/include/Common/*.hpp")

# Core source files
# file(GLOB_RECURSE CORE_SRC
# 	"${ROOT_DIR}/src/Core/*.cpp"
# 	"${ROOT_DIR}/src/Core/*.hpp"
# )

# Core include files
# file(GLOB_RECURSE CORE_INC "${ROOT_DIR}/include/Core/*.hpp")

# Windows source files
# file(GLOB_RECURSE WINDOWS_SRC
#     "${ROOT_DIR}/src/Win/*.cpp"
#     "${ROOT_DIR}/src/Win/*.hpp"
# )

# Windows include files
# file(GLOB_RECURSE WINDOWS_INC "${ROOT_DIR}/include/Win/*.hpp")

# DirectX source files
# file(GLOB_RECURSE DX_SRC
#     "${ROOT_DIR}/src/DX/*.cpp"
#     "${ROOT_DIR}/src/DX/*.hpp"
# )

# DirectX include files
# file(GLOB_RECURSE DX_INC "${ROOT_DIR}/include/DX/*.hpp")

# Vertex shaders
# file(GLOB_RECURSE VERTEX_SHADERS "${ROOT_DIR}/resources/shaders/*VS.hlsl")

# Pixel shaders
# file(GLOB_RECURSE PIXEL_SHADERS "${ROOT_DIR}/resources/shaders/*PS.hlsl")

# source_group("CMEngine/Source Files/" FILES ${CMENGINE_SRC})
# source_group("CMEngine/Include Files/" FILES ${CMENGINE_INC})
# 
# source_group("CMEngine/Source Files/Common" FILES ${COMMON_SRC})
# source_group("CMEngine/Include Files/Common" FILES ${COMMON_INC})
# 
# source_group("CMEngine/Source Files/Core" FILES ${CORE_SRC})
# source_group("CMEngine/Include Files/Core" FILES ${CORE_INC})
# 
# source_group("CMEngine/Source Files/Win" FILES ${WINDOWS_SRC})
# source_group("CMEngine/Include Files/Win" FILES ${WINDOWS_INC})
# 
# source_group("CMEngine/Source Files/DX" FILES ${DX_SRC})
# source_group("CMEngine/Include Files/DX" FILES ${DX_INC})
# 
# source_group("CMEngine/Shaders/Vertex" FILES ${VERTEX_SHADERS})
# source_group("CMEngine/Shaders/Pixel" FILES ${PIXEL_SHADERS})

# set(SHADER_FILES ${VERTEX_SHADERS} ${PIXEL_SHADERS})

# set_source_files_properties(${VERTEX_SHADERS} PROPERTIES ShaderType "vs")
# set_source_files_properties(${PIXEL_SHADERS} PROPERTIES ShaderType "ps")
# set_source_files_properties(${SHADER_FILES} PROPERTIES ShaderModel "4_0")


# message(STATUS "CMEngine Binary Dir: ${CMAKE_CURRENT_BINARY_DIR}")

# set(SHADER_OUTPUT_DIR ${CMENGINE_OUTPUT_DIR})

# Prepare a list of compiled shader outputss
# set(COMPILED_SHADERS)

# foreach(FILE ${SHADER_FILES})
#     get_filename_component(FILE_WE ${FILE} NAME_WE)
#     get_source_file_property(shadertype ${FILE} ShaderType)
#     get_source_file_property(shadermodel ${FILE} ShaderModel)
# 
#     set(CSO_OUTPUT "${SHADER_OUTPUT_DIR}/${FILE_WE}.cso")
#     set(PDB_OUTPUT "${SHADER_OUTPUT_DIR}/${FILE_WE}.pdb")
# 
#     add_custom_command(
#         OUTPUT ${CSO_OUTPUT} ${PDB_OUTPUT}
#         COMMAND ${FXC_EXECUTABLE}
#                 /nologo /Emain
#                 /T${shadertype}_${shadermodel}
#                 $<IF:$<CONFIG:DEBUG>,/Od,/O1>
#                 /Zi
#                 /Fo ${CSO_OUTPUT}
#                 /Fd ${PDB_OUTPUT}
#                 ${FILE}
#         MAIN_DEPENDENCY ${FILE}
#         COMMENT "Compiling HLSL shader: ${FILE}"
#         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#         VERBATIM
#     )
# 
#     list(APPEND COMPILED_SHADERS ${CSO_OUTPUT})
# endforeach()

# Create a buildable target from shader outputs
# add_custom_target(CMEngineShaders ALL DEPENDS ${COMPILED_SHADERS})

# add_executable(CMEngine 
#     ${MAIN_SRC}
#     ${CMENGINE_SRC}
#     ${CMENGINE_INC}
#     ${COMMON_SRC}
#     ${COMMON_INC}
#     ${CORE_SRC}
#     ${CORE_INC}
#     ${WINDOWS_SRC}
#     ${WINDOWS_INC}
#     ${DX_SRC}
#     ${DX_INC}
#     ${PIXEL_SHADERS}
#     ${VERTEX_SHADERS}
# )

# add_dependencies(CMEngine CMEngineShaders)

# target_precompile_headers(CMEngine PRIVATE "include/Core/PCH.hpp")

# set(CMAKE_CXX_FLAGS_DEBUG CACHE STRING "/DCM_DEBUG=1" FORCE)
# set(CMAKE_CXX_FLAGS_RELEASE CACHE STRING "/DCM_NDEBUG=1 /DCM_RELEASE=1" FORCE)
# set(CMAKE_CXX_FLAGS_DIST CACHE STRING "/DCM_NDEBUG=1 /DCM_DIST=1" FORCE)

# target_include_directories(CMEngine PUBLIC
#     "${CMAKE_SOURCE_DIR}/CMEngine/include"
#     "${CMAKE_SOURCE_DIR}/vendor/CMDep_DearImGui"
#     "${CMAKE_SOURCE_DIR}/vendor/CMDep_yaml-cpp/include"
# )

# TODO: Figure out DirectXTK's CMake integration. Turn the current CMDep's into submodules.

# target_link_libraries(CMEngine PRIVATE
#     CMDep_DearImGui
#     CMDep_yaml-cpp
# )