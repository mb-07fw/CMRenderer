#include "CMPlatform/IGraphics.hpp"

namespace CMEngine::Platform
{
	IGraphics::IGraphics(const GraphicsFuncTable& funcTable) noexcept
		: m_FuncTable(funcTable)
	{
	}

	void IGraphics::Clear(RGBANorm color) noexcept
	{
		m_FuncTable.Clear(color);
	}

	void IGraphics::Present() noexcept
	{
		m_FuncTable.Present();
	}

	void IGraphics::Draw(const void* pBuffer, const DrawDescriptor* pDescriptor) noexcept
	{
		m_FuncTable.Draw(pBuffer, pDescriptor);
	}
}