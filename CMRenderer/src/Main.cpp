#include "Core/CMPCH.hpp"
#include "CMRenderer.hpp"

void Entry()
{
	CMRenderer::CMWindowData windowSettings(L"TITLE!!!", false, 640, 480);
	CMRenderer::CMRendererSettings rendererSettings(windowSettings);

	{
		CMRenderer::CMRenderer renderer(rendererSettings);

		renderer.Init();
		renderer.Run();
		renderer.Shutdown();
	}
}

#if defined(CM_DEBUG)

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