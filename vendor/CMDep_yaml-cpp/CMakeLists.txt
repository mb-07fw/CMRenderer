# CMDep_yaml-cpp CMakeLists.txt

set(ROOT_DIR "${CMAKE_SOURCE_DIR}/vendor/CMDep_yaml-cpp")

file(GLOB_RECURSE SRC "${ROOT_DIR}/src/*.cpp")
file(GLOB_RECURSE HEADERS "${ROOT_DIR}/src/*.h")
file(GLOB_RECURSE PUBLIC_HEADERS "${ROOT_DIR}/include/*.h")

source_group("CMDep_yaml-cpp/Include Files/Core" FILES ${PUBLIC_HEADERS})
source_group("CMDep_yaml-cpp/Source Files/Core" FILES ${SRC} ${HEADERS})

add_library(CMDep_yaml-cpp STATIC ${SRC} ${HEADERS} ${PUBLIC_HEADERS})

target_include_directories(CMDep_yaml-cpp PUBLIC
    "${CMAKE_SOURCE_DIR}/vendor/CMDep_yaml-cpp/include/"
)

target_compile_definitions(CMDep_yaml-cpp PUBLIC YAML_CPP_STATIC_DEFINE)

SetTargetCommonWarnings(CMDep_yaml-cpp)

set_target_properties(CMDep_yaml-cpp PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/$<CONFIG>/vendor/CMDep_yaml-cpp/out"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/$<CONFIG>/vendor/CMDep_yaml-cpp/out"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/$<CONFIG>/vendor/CMDep_yaml-cpp/int"
)

# Disable due to yaml-cpp using a part of the C++ STL
#   (std::pair) which results in warnings on higher
#   warning levels.
if (MSVC)
    target_compile_options(CMDep_yaml-cpp PRIVATE /wd4244)
endif()