#include "Core/CME_PCH.hpp"
#include "Core/CME_Engine.hpp"

void Entry()
{
	CMEngine::CMEngine engine;
	engine.Run();
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
