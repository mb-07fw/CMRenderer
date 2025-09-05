# CMSandbox CMakeLists.txt
set(ROOT_DIR "${CMAKE_SOURCE_DIR}/CMSandbox")

set(SRC_FILES
    "${ROOT_DIR}/src/Main.cpp"
)

add_executable(CMSandbox ${SRC_FILES})

target_link_libraries(CMSandbox PRIVATE CMEngine)

set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/CMSandbox/out")
SetTargetCommon(CMSandbox ${OUTPUT_DIR})

add_custom_command(TARGET CMSandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:CMEngine>"
        "${OUTPUT_DIR}"  # destination folder
)