#pragma once

#include "IWindow.hpp"
#include "IGraphics.hpp"

namespace CMPlatform
{
	class CM_PLATFORM_API IPlatform
	{
	public:
		virtual void Update() noexcept = 0;

		inline [[nodiscard]] IWindow* Window() noexcept { return mP_Window; }
		inline [[nodiscard]] IGraphics* Graphics() noexcept { return mP_Graphics; }
	protected:
		IPlatform(
			IWindow* pWindow,
			IGraphics* pGraphics
		) noexcept;

		virtual ~IPlatform() = default;

		IPlatform(const IPlatform& other) = delete;
		IPlatform& operator=(const IPlatform& other) = delete;
	private:
		IWindow* mP_Window;
		IGraphics* mP_Graphics;
	};
}