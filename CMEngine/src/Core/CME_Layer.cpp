#include "Core/CME_PCH.hpp"
#include "Core/CME_Layer.hpp"

namespace CMEngine
{
	ICMLayer::ICMLayer(CMCommon::CMLoggerWide& logger, DirectXAPI::DX11::DXRenderer& renderer) noexcept
		: m_Logger(logger),
		  m_Renderer(renderer)
	{
	}
}