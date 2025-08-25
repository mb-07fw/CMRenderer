# CMImGui CMakeLists.txt
set(SRC_DIR "src/")
set(INCLUDE_DIR "include/")
set(MAIN_FILES
	"${SRC_DIR}/imgui/imconfig.h"
	"${SRC_DIR}/imgui/imgui_internal.h"
	"${SRC_DIR}/imgui/imstb_rectpack.h"
	"${SRC_DIR}/imgui/imstb_truetype.h"
	"${SRC_DIR}/imgui/imstb_textedit.h"
	"${SRC_DIR}/imgui/imgui.h"
	"${SRC_DIR}/imgui/imgui.cpp"
	"${SRC_DIR}/imgui/imgui_draw.cpp"
	"${SRC_DIR}/imgui/imgui_widgets.cpp"
	"${SRC_DIR}/imgui/imgui_tables.cpp"
	"${INCLUDE_DIR}/imgui/CMImGui.hpp"
)

set(DEFINES)
set(BACKEND_FILES)

if (WIN32)
	if (CM_IMGUI_BUILD_DX12)
		list(APPEND BACKEND_FILES "${SRC_DIR}/backends/imgui_impl_dx12.cpp" "${SRC_DIR}/backends/imgui_impl_dx12.h")
		list(APPEND DEFINES CM_IMGUI_BUILD_DX12=${CM_IMGUI_BUILD_DX12})
	endif()

	if (CM_IMGUI_BUILD_GLFW)
		AddGLFW()
	endif()

	list(APPEND BACKEND_FILES
		"${SRC_DIR}/imgui/backends/imgui_impl_win32.cpp"
		"${SRC_DIR}/imgui/backends/imgui_impl_win32.h"
		"${SRC_DIR}/imgui/backends/imgui_impl_dx11.cpp"
		"${SRC_DIR}/imgui/backends/imgui_impl_dx11.h"
	)
else()

	if (BUILD_DX12)
		message(WARNING "CMImGui: `BUILD_DX12` is defined, but DirectX 12 is not available on non-Windows platforms.")
	endif()

	AddGLFW()
endif()

function(AddGLFW)
	list(APPEND BACKEND_FILES "${SRC_DIR}/backends/imgui_impl_glfw.cpp" "${SRC_DIR}/backends/imgui_impl_glfw.h")
	list(APPEND DEFINES CM_IMGUI_BUILD_GLFW=${CM_IMGUI_BUILD_GLFW})
endfunction()

add_library(CMImGui STATIC ${MAIN_FILES} ${BACKEND_FILES})

target_include_directories(CMImGui PUBLIC
	${SRC_DIR}
	${INCLUDE_DIR}
	PRIVATE "${SRC_DIR}/imgui"
)

target_compile_definitions(CMImGui PUBLIC ${DEFINES})