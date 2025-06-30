#pragma once

#include "DirectX/CME_DXRenderer.hpp"
#include "CMC_Logger.hpp"

namespace CMEngine
{
	class ICMLayer
	{
	public:
		ICMLayer(CMCommon::CMLoggerWide& logger, DirectXAPI::DX11::DXRenderer& renderer) noexcept;
		virtual ~ICMLayer() = default;
	public:
		virtual void OnAttach() noexcept = 0;
		virtual void OnDetach() noexcept = 0;
		virtual void OnUpdate(float deltaTime) noexcept = 0;
	protected:
		CMCommon::CMLoggerWide& m_Logger;
		DirectXAPI::DX11::DXRenderer& m_Renderer;
	private:
	};
}