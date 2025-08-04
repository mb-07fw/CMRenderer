# CMPlatform_Core CMakeLists.txt
set(ROOT_DIR "${CMAKE_SOURCE_DIR}/CMEngine/CMPlatform_Core")

set(PLATFORM_FILES
    "${ROOT_DIR}/include/Platform/Export.hpp"
    "${ROOT_DIR}/include/Platform/IPlatform.hpp"
    "${ROOT_DIR}/include/Platform/LoadPlatform.hpp"
    "${ROOT_DIR}/include/Platform/Platform.hpp"
    "${ROOT_DIR}/include/Platform/PlatformDef.hpp"

    "${ROOT_DIR}/src/Platform/LoadPlatform.cpp"
    "${ROOT_DIR}/src/Platform/IPlatform.cpp"
    "${ROOT_DIR}/src/Platform/IWindow.cpp"
)

add_library(CMPlatform_Core STATIC ${PLATFORM_FILES})
target_link_libraries(CMPlatform_Core PUBLIC spdlog::spdlog)

target_include_directories(CMPlatform_Core 
    PUBLIC "${ROOT_DIR}/include"
    PUBLIC "${CMAKE_SOURCE_DIR}/vendor/submodules/spdlog/include"
    PRIVATE "${ROOT_DIR}/src"
)
target_compile_definitions(CMPlatform_Core
    PRIVATE CM_BUILD
    PUBLIC UNICODE
    PUBLIC _UNICODE
)

set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/build/$<CONFIG>/CMEngine/CMPlatform_Core/out")

set_target_properties(CMPlatform_Core PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
    ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}
)

SetTargetCommonWarnings(CMPlatform_Core)