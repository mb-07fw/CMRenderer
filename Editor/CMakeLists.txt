# Editor CMakeLists.txt

set(Src_Files 
    "src/Main.cpp"

    "src/Editor.cpp"
    "src/Editor.hpp"
)

add_executable(Editor ${Src_Files})

target_link_libraries(Editor
    PRIVATE Engine
    PRIVATE Common
)

set(Output_Dir "${CMAKE_BINARY_DIR}/Editor/out")

SetCommon(Editor ${Output_Dir})

add_dependencies(Editor Platform)

add_custom_command(TARGET Editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:Platform>"
        "${Output_Dir}"
)