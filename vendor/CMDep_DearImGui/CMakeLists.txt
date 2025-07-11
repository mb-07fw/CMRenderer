# CMDep_DearImGui CMakeLists.txt

set(ROOT_DIR "${CMAKE_SOURCE_DIR}/vendor/CMDep_DearImGui/imgui")

file(GLOB_RECURSE SRC "${ROOT_DIR}/*.cpp")
file(GLOB_RECURSE HEADERS "${ROOT_DIR}/*.hpp")

source_group("CMDep_DearImGui/Header Files/Core" FILES ${HEADERS})
source_group("CMDep_DearImGui/Source Files/Core" FILES ${SRC})

add_library(CMDep_DearImGui STATIC ${SRC} ${HEADERS})

target_include_directories(CMDep_DearImGui PUBLIC
    "${CMAKE_SOURCE_DIR}/vendor/CMDep_DearImGui/imgui/"
)

SetTargetCommonWarnings(CMDep_DearImGui)

set_target_properties(CMDep_DearImGui PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/vendor/CMDep_DearImGui/out
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/vendor/CMDep_DearImGui/out
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/vendor/CMDep_DearImGui/int
)