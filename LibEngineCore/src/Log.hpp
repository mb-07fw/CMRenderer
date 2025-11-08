#pragma once

#include "Macros.hpp"

#include <spdlog/spdlog.h>

#define CM_ENGINE_LOG_INFO(...) CM_ENGINE_IF_DEBUG(SPDLOG_INFO(__VA_ARGS__))
#define CM_ENGINE_LOG_WARN(...) SPDLOG_WARN(__VA_ARGS__)
#define CM_ENGINE_LOG_CRITICAL(...) SPDLOG_CRITICAL(__VA_ARGS__)