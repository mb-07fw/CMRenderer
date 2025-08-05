#include "CMPlatform/IPlatform.hpp"

#include <spdlog/spdlog.h>

namespace CMEngine::Platform
{
	IPlatform::IPlatform(
		IWindow* pWindow,
		IGraphics* pGraphics,
		const PlatformFuncTable& funcTable
	) noexcept
		: mP_Window(pWindow),
		  mP_Graphics(pGraphics),
		  m_FuncTable(funcTable)
	{
		VerifyFuncTable();
	}

	void IPlatform::Update() noexcept
	{
		m_FuncTable.Update();
	}

	void IPlatform::VerifyFuncTable() const noexcept
	{
		if (!m_FuncTable.Update)
		{
			spdlog::error("IPlatform internal error: Update function pointer is null.");
			std::exit(-1);
		}
	}
}