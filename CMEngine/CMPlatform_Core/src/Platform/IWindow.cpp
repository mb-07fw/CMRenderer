#include "Platform/IWindow.hpp"

namespace CMEngine::Platform
{
	IWindow::IWindow(const WindowFuncTable& funcTable) noexcept
		: m_FuncTable(funcTable)
	{
	}
}