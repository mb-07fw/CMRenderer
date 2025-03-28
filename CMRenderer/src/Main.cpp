#include "Core/CMPCH.hpp"

#include "CMRenderer.hpp"

void Entry()
{
	/* !!! WARNING:
	 *		Dont pass true for useFullscreen is setting breakpoints, as breaking the debugger with a maximized window will hijack your display,
	 *		and require restarting your computer to fix. If a breakpoint is triggered when the window is maximized, 
	 *		the window will become unresponsive and unable to be minimized.
	 * 
	 * !!! */
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