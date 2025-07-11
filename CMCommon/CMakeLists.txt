# CMCommon CMakeLists.txt

# Glob all .cpp files in src/
file(GLOB SRC_FILES "${CMAKE_SOURCE_DIR}/CMCommon/src/*.cpp")

# Glob all .hpp files in src/
file(GLOB HEADER_FILES "${CMAKE_SOURCE_DIR}/CMCommon/src/*.hpp")

# Glob all .hpp files in include/
file(GLOB PUBLIC_HEADER_FILES "${CMAKE_SOURCE_DIR}/CMCommon/include/*.hpp")

source_group("CMCommon/Source Files" FILES ${SRC_FILES} ${HEADER_FILES})
source_group("CMCommon/Header Files" FILES ${PUBLIC_HEADER_FILES})

add_library(CMCommon STATIC ${SRC_FILES} ${HEADER_FILES} ${PUBLIC_HEADER_FILES})

target_include_directories(CMCommon PUBLIC "${CMAKE_SOURCE_DIR}/CMCommon/include")

SetTargetCommonWarnings(CMCommon)
SetTargetBuildDirectories(CMCommon)