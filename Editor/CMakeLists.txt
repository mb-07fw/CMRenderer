# Editor CMakeLists.txt
set(SRC_FILES
    "src/Main.cpp"
    "src/Editor.hpp"
    "src/Editor.cpp"
)

add_executable(Editor ${SRC_FILES})

target_link_libraries(Editor PRIVATE LibEngineCore)

set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/Editor/out")
SetTargetCommon(Editor ${OUTPUT_DIR})

add_custom_command(TARGET Editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:LibEngineCore>"
        "${OUTPUT_DIR}"  # destination folder
)