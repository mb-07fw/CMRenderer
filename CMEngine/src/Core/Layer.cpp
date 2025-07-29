#include "Core/PCH.hpp"
#include "Core/Layer.hpp"

namespace CMEngine::Core
{
	ILayer::ILayer(
		Common::LoggerWide& logger,
		DX::DX11::Renderer& renderer,
		Win::Window& window
	) noexcept
		: m_Logger(logger),
		  m_Renderer(renderer),
		  m_Window(window)
	{
	}
}