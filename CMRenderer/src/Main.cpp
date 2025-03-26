#include "Core/CMPCH.hpp"

#define CM_NO_VULKAN
#define CM_NO_OPENGL

#include "CMRenderer.hpp"

void Entry()
{
	/* 
	 * !!! WARNING:
	 *		Dont pass true for useFullscreen, as breaking the debugger with a maximized window will hijack your display,
	 *		and require restarting your computer to fix. Currently, __debugbreak is being used for any fatal logs, so if
	 *		a problem is found at runtime, the window will become unresponsive and unable to be minimized.
	 * !!!
	 */
	CMRenderer::CMWindowData windowSettings(L"TITLE!!!", false, 640, 480);
	CMRenderer::CMRendererSettings rendererSettings(windowSettings);

	{
		CMRenderer::CMRenderer renderer(rendererSettings);

		renderer.Init();
		renderer.Run();
		renderer.Shutdown();
	}
}

#ifndef CM_DIST

int main()
{
	

	Entry();
}

#else

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd
)
{
	Entry();
}

#endif