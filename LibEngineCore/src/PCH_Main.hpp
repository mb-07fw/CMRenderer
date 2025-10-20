#pragma once

#include <cstdlib>
#include <cstdint>

#include <iostream>
#include <string>
#include <string_view>
#include <array>
#include <vector>
#include <span>
#include <memory>
#include <map>
#include <unordered_map>
#include <filesystem>
#include <type_traits>
#include <limits>
#include <functional>

#include <spdlog/spdlog.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/callback_sink.h>
#include "spdlog/sinks/basic_file_sink.h"

#include <imgui/CMImGui.hpp>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags