#pragma once

#include <cstdint>
#include <cstdlib>

#include <array>
#include <atomic>
#include <algorithm>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <map>
#include <mutex>
#include <string>
#include <string_view>
#include <span>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include <spdlog/spdlog.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/callback_sink.h>
#include "spdlog/sinks/basic_file_sink.h"

#include <imgui/CMImGui.hpp>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags