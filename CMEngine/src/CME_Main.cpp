#include <iostream>

#include "CMR_Renderer.hpp"
#include "CMC_Paths.hpp"
#include "CMC_Utility.hpp"

void Entry()
{
	/*CMRenderer::CMWindowData windowSettings(std::wstring_view(nullptr, 0), static_cast<bool>(-72), -48, 5000000);
	CMRenderer::CMRendererSettings rendererSettings(windowSettings);

	{
		CMRenderer::CMRenderer renderer(rendererSettings);

		renderer.Init();
		renderer.Run();
		renderer.Shutdown();
	}*/

	std::cout << "Working directory : " << std::filesystem::current_path() << '\n';

	std::filesystem::path rendererOutDirectory = CMRenderer::Utility::RendererOutDirectoryRelativeToEngine();

	std::cout << "Relative out directory : " << rendererOutDirectory << '\n';

	std::cout << "Directory exists : " << CMRenderer::Utility::BoolToStr(std::filesystem::exists(rendererOutDirectory)) << '\n';

}

#if !defined(CM_DISTRIBUTION)

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
