#include "Core/PCH.hpp"
#include "Engine.hpp"

#if !defined(CM_DISTRIBUTION)

#define EntryPoint int main()

#else

#define EntryPoint int WINAPI WinMain( \
	_In_ HINSTANCE hInstance, \
	_In_opt_ HINSTANCE hPrevInstance, \
	_In_ LPSTR lpCmdLine, \
	_In_ int nShowCmd \
)
{
	Entry();
}

#endif

EntryPoint
{
	CMEngine::CMEngine engine;

	engine.Init();
	engine.Run();
}