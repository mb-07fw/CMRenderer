#pragma once

/* This file is a custom integration of DearImGui. */

#include "imgui/imgui.h"

#ifdef _WIN32
	#include "imgui/backends/imgui_impl_win32.h"
	#include "imgui/backends/imgui_impl_dx11.h"

#ifdef CM_IMGUI_BUILD_DX12
	#include "imgui/backends/imgui_impl_dx12.h"
#endif

#endif

#ifdef CM_IMGUI_BUILD_GLFW
	#include "imgui/backends/imgui_impl_glfw.h"
#endif