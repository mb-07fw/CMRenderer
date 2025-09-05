# CMPlatform CMakeLists.txt
set(ROOT_DIR "${CMAKE_SOURCE_DIR}/modules/CMPlatform")

add_library(CMPlatform SHARED 
    "${ROOT_DIR}/include/CMPlatform.hpp"
    "${ROOT_DIR}/src/CMPlatform.cpp"
)

# Set this because linker cannot infer the language if there's no translation units for compilation.
set_target_properties(CMPlatform PROPERTIES LINKER_LANGUAGE CXX)

if (WIN32)
    add_subdirectory(
        "${ROOT_DIR}/WinImpl"
        "${CMAKE_BINARY_DIR}/modules/CMPlatform/WinImpl"
    )
    
    # Link publicly so that CMPlatform_Core is properly propagated through CMPlatform -> CMEngine.
    target_link_libraries(CMPlatform PUBLIC CMPlatform_WinImpl)
endif()

target_include_directories(CMPlatform PUBLIC "${ROOT_DIR}/include")

set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/modules/CMPlatform/CMPlatform")
SetTargetCommon(CMPlatform ${OUTPUT_DIR})