#include "CMPlatform/IGraphics.hpp"

namespace CMEngine::Platform
{
	IGraphics::IGraphics(const GraphicsFuncTable& funcTable) noexcept
		: m_FuncTable(funcTable)
	{
	}

	void IGraphics::Clear(ColorNorm color) noexcept
	{
		m_FuncTable.Clear(color);
	}

	void IGraphics::Present() noexcept
	{
		m_FuncTable.Present();
	}
}