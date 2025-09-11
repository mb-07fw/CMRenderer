#pragma once

#include "Export.hpp"
#include "Platform/Core/IPlatform.hpp"
#include "Platform/WinImpl/Window_WinImpl.hpp"
#include "Platform/WinImpl/Graphics_WinImpl.hpp"

namespace CMEngine::Platform::WinImpl
{
	class CM_ENGINE_API Platform : public IPlatform
	{
	public:
		inline Platform() noexcept
			: m_Graphics(m_Window)
		{
			SetInterfaces(m_Window, m_Graphics);
		}

		~Platform() = default;

		inline virtual bool Update() noexcept override
		{
			if (m_Window.ShouldClose())
				return false;

			m_Window.Update();
			m_Graphics.Update();
		}

		inline virtual [[nodiscard]] bool IsRunning() const noexcept override { return !m_Window.ShouldClose(); }
	private:
		Window m_Window;
		Graphics m_Graphics;
	};
}