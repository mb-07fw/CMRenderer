# CMPlatform_Core CMakeLists.txt

# TODO: Make CMPlatform_Core a shared library which is load-time linked against by both CMSandbox and CMPlatform_WinImpl to enable safer
#         polymorphism as the .lib approach for both sides allowed different vtable symbols for types defined in CMPlatform_Core.

set(PARENT_DIR "${CMAKE_SOURCE_DIR}/modules/CMPlatform")
set(PARENT_BINARY_DIR "${CMAKE_BINARY_DIR}/modules/CMPlatform")
set(ROOT_DIR "${PARENT_DIR}/Core")

set(INCLUDE_DIR "${ROOT_DIR}/include")
set(SRC_DIR "${ROOT_DIR}/src")

set(PLATFORM_FILES
    "${INCLUDE_DIR}/CMPlatform/Export.hpp"
    "${INCLUDE_DIR}/CMPlatform/IPlatform.hpp"
    "${INCLUDE_DIR}/CMPlatform/LoadPlatform.hpp"
    "${INCLUDE_DIR}/CMPlatform/Platform.hpp"
    "${INCLUDE_DIR}/CMPlatform/PlatformTypes.hpp"
    "${INCLUDE_DIR}/CMPlatform/Types.hpp"

    "${SRC_DIR}/CMPlatform/LoadPlatform.cpp"
    "${SRC_DIR}/CMPlatform/IPlatform.cpp"
    "${SRC_DIR}/CMPlatform/IWindow.cpp"
    "${SRC_DIR}/CMPlatform/IGraphics.cpp"
    "${SRC_DIR}/CMPlatform/Types.cpp"
)

add_library(CMPlatform_Core STATIC ${PLATFORM_FILES})

add_subdirectory(
    "${CMAKE_SOURCE_DIR}/modules/CMShaders"
    "${CMAKE_BINARY_DIR}/modules/CMShaders"
)

# Depend on CMShaders target, and inherit CMShaders defines.
add_dependencies(CMPlatform_Core CMShaders)
target_link_libraries(CMPlatform_Core PUBLIC CMShadersConfig)

target_link_libraries(CMPlatform_Core PUBLIC spdlog::spdlog)

# modules/CMPlatform/Core/include/...whatever_file_here...
target_include_directories(CMPlatform_Core 
    PUBLIC "${INCLUDE_DIR}"
    PUBLIC "${CMAKE_SOURCE_DIR}/vendor/submodules/spdlog/include"
    PRIVATE "${SRC_DIR}"
)

target_compile_definitions(CMPlatform_Core
    PRIVATE CM_BUILD
    PUBLIC UNICODE
    PUBLIC _UNICODE
    PUBLIC $<$<CONFIG:Debug>:CM_PLATFORM_DEBUG=1>
)

set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/build/$<CONFIG>/modules/CMPlatform/Core/out")

set_target_properties(CMPlatform_Core PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
    ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}
)

SetTargetCommonWarnings(CMPlatform_Core)

if (WIN32)
    add_subdirectory(
        "${PARENT_DIR}/WinImpl"
        "${PARENT_BINARY_DIR}/WinImpl"
    )

    # Ensure that CMPlatform/Core is built first.
    add_dependencies(CMPlatform_WinImpl CMPlatform_Core)

    # Force build regardless of linkage
    add_custom_target(BuildWinImpl ALL
        DEPENDS CMPlatform_WinImpl
    )

    target_compile_definitions(CMPlatform_Core PRIVATE
        CM_PLATFORM_PATH_WIN_IMPL_DLL="$<TARGET_FILE:CMPlatform_WinImpl>"
        CM_PLATFORM_PATH_WIN_IMPL_DLLW=L"$<TARGET_FILE:CMPlatform_WinImpl>"
    )
endif()