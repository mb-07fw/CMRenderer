# CMSandbox CMakeLists.txt
set(ROOT_DIR "${CMAKE_SOURCE_DIR}/CMSandbox")

set(MAIN_SRC "${ROOT_DIR}/src/Main.cpp")

add_executable(CMSandbox ${MAIN_SRC})

target_link_libraries(CMSandbox PRIVATE CMEngine)

set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/build/$<CONFIG>/CMSandbox/out")

set_target_properties(CMSandbox PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
    ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}
)

add_custom_command(TARGET CMSandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:CMEngine>"
        "${OUTPUT_DIR}"  # destination folder
)

if (WIN32)

add_dependencies(CMSandbox CMPlatform_WinImpl)

# add_custom_command(TARGET CMSandbox POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_if_different
#         "$<TARGET_FILE:CMPlatform_WinImpl>"
#         "${OUTPUT_DIR}"  # destination folder
# )

endif()

SetTargetCommonWarnings(CMSandbox)

# We want to make sure the platform builds but we don't link to it in favor of loading it dynamically.
# add_dependencies(CMSandbox CMPlatform)