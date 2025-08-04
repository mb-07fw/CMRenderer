#pragma once

#include "DX/DX11/DX11_Renderer.hpp"
#include "Win/Win_Window.hpp"
#include "Common/Logger.hpp"

namespace CMEngine::Core
{
	class ILayer
	{
	public:
		ILayer(
			Common::LoggerWide& logger,
			DX::DX11::Renderer& renderer,
			Win::Window& window
		) noexcept;
		virtual ~ILayer() = default;
	public:
		virtual void OnAttach() noexcept = 0;
		virtual void OnDetach() noexcept = 0;
		virtual void OnUpdate(float deltaTime) noexcept = 0;
	protected:
		Common::LoggerWide& m_Logger;
		DX::DX11::Renderer& m_Renderer;
		Win::Window& m_Window;
	};
}