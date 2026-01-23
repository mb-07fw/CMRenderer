# Common CMakeLists.txt

set(Build_Files 
    "Dummy.cpp" # to ensure there is a .lib to link to...

    "Assert.hpp"
    "Cast.hpp"
    "Config.hpp"
    "IBasicLogger.hpp"
    "Math.hpp"
    "RuntimeFailure.hpp"
    "View.hpp"
)

add_library(Common STATIC ${Build_Files})

set(Output_Dir "${CMAKE_BINARY_DIR}/Common/out")

target_include_directories(Common PUBLIC "${CMAKE_SOURCE_DIR}")

SetCommon(Common ${Output_Dir})

target_compile_features(Common PUBLIC cxx_std_20)