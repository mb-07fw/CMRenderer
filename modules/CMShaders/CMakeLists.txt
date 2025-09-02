# CMShaders CMakeLists.txt

# Note for me in the future, user define is different from FXC_EXECUTABLE as CMake
#   remembers find_program through the cache, and was causing it to use the x86
#   version of FXC as it was always the first it found outside of Visual Studio's CMake.
if (DEFINED FXC_EXECUTABLE_USER AND EXISTS "${FXC_EXECUTABLE_USER}")
    message(STATUS "Using user-specified FXC: ${FXC_EXECUTABLE_USER}")
    set(FXC_EXECUTABLE_USER "" CACHE FILEPATH "Optional user-specified path to FXC executable.")
    set(FXC_EXECUTABLE ${FXC_EXECUTABLE_USER})
else()
    if(DEFINED FXC_EXECUTABLE_USER AND NOT EXISTS "${FXC_EXECUTABLE_USER}")
        message(STATUS "FXC_EXECUTABLE_USER was defined, but doesn't exist.")
    endif()

    # Note for anybody else struggling with finding FXC, you may needs to manually 
    #   set the HINTS path to match your system's locatation of FXC.
    find_program(FXC_EXECUTABLE fxc
        DOC "Path to the FX Compiler (fxc)"
    )

    if (FXC_EXECUTABLE)
        message(STATUS "Found FXC in PATH: ${FXC_EXECUTABLE}")
    else()
        message(STATUS "Failed to find FXC in PATH.")
    endif()

    if (FXC_EXECUTABLE MATCHES [[/x64/]])
        message(STATUS "FXC is already x64: ${FXC_EXECUTABLE}")
    else()
        # Try to search C: drive instead.
        # (Currently only Program Files (x86) where Visual Studio 17 2022 installs it)

        if (FXC_EXECUTABLE)
            message(STATUS "Searching for FXC in Program Files (x86) again. (A x86 FXC was found instead of an x64)")
        else()
            message(STATUS "Searching for FXC in Program Files (x86).")
        endif()

        # Extract the environment variable into a proper CMake variable for use in glob-ing.
        set(PFX86 "$ENV{ProgramFiles\(x86\)}")

        message(STATUS "PF: ${PFX86}")

        # Search for "fxc.exe" in any subdirectory under Program Files (x86)\Windows Kits\10\bin\
        file(GLOB_RECURSE FXC_CANDIDATES "${PFX86}/Windows Kits/10/bin/*/x64/fxc.exe")

        list(LENGTH FXC_CANDIDATES NUM_FXC_CANDIDATES)

        foreach(path IN LISTS FXC_CANDIDATES)
            message(STATUS "Candidate FXC: ${path}")
        endforeach()

        if (NUM_FXC_CANDIDATES GREATER 0)
            foreach(path IN LISTS FXC_CANDIDATES)
                if("${path}" MATCHES [[/x64/]])
                    set(FXC_EXECUTABLE "${path}")
                    break()
                endif()
            endforeach()
        else()
            message(FATAL_ERROR
                "FXC (Effect-Compiler Tool) not found in Program Files (x86). Compiled shaders will not be present."
                "You may use `-DFXC_EXECUTABLE_USER=\"...\" as a supplimentary path to a custom x64 FXC install instead."
            )
        endif()

        message(STATUS "Final FXC Candidate: ${FXC_EXECUTABLE}")
    endif()
endif()

set(ROOT_DIR "${CMAKE_SOURCE_DIR}/modules/CMShaders")

set(VERTEX_SHADERS "${ROOT_DIR}/Quad_VS.hlsl")
set(PIXEL_SHADERS "${ROOT_DIR}/Quad_PS.hlsl")

set(SHADER_FILES ${VERTEX_SHADERS} ${PIXEL_SHADERS})

set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/modules/CMShaders/out")

message(STATUS "CMShaders output directory: ${OUTPUT_DIR}")

set_source_files_properties(${VERTEX_SHADERS} PROPERTIES ShaderType "vs")
set_source_files_properties(${PIXEL_SHADERS} PROPERTIES ShaderType "ps")
set_source_files_properties(${SHADER_FILES} PROPERTIES ShaderModel "4_0")

# Prepare a list of compiled shader outputss
set(COMPILED_SHADERS)

foreach(FILE ${SHADER_FILES})
    if (NOT EXISTS ${FILE})
        message(FATAL_ERROR "Shader file doesn't exist: ${FILE}")
    endif()

    get_filename_component(FILE_WE ${FILE} NAME_WE)
    get_source_file_property(shader_type ${FILE} ShaderType)
    get_source_file_property(shader_model ${FILE} ShaderModel)

    set(CSO_OUTPUT "${OUTPUT_DIR}/${FILE_WE}.cso")
    set(PDB_OUTPUT "${OUTPUT_DIR}/${FILE_WE}.pdb")

    add_custom_command(
        OUTPUT ${CSO_OUTPUT} ${PDB_OUTPUT}
        COMMAND ${CMAKE_COMMAND} -E echo
        "FXC: ${FXC_EXECUTABLE} /nologo /Emain /T${shader_type}_${shader_model} $<IF:$<CONFIG:DEBUG>,/Od,/O1> /Zi /Fo ${CSO_OUTPUT} /Fd ${PDB_OUTPUT} ${FILE}"
        COMMAND ${FXC_EXECUTABLE}
                /nologo /Emain
                /T${shader_type}_${shader_model}
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
add_custom_target(CMShaders ALL DEPENDS ${COMPILED_SHADERS})

set_target_properties(CMShaders PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
    ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}
)

add_library(CMShadersConfig INTERFACE)

target_compile_definitions(CMShadersConfig INTERFACE
    CM_PLATFORM_PATH_SHADER_DIRECTORY="${ROOT_DIR}"
    CM_PLATFORM_PATH_SHADER_DIRECTORYW=L"${ROOT_DIR}"
    CM_PLATFORM_PATH_COMPILED_SHADER_DIRECTORY="${OUTPUT_DIR}"
    CM_PLATFORM_PATH_COMPILED_SHADER_DIRECTORYW=L"${OUTPUT_DIR}"
)