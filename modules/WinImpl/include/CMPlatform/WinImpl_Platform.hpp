#pragma once


#include "CMPlatform/IPlatform.hpp"
#include "CMPlatform/Export.hpp"

#include "CMPlatform/WinImpl_Window.hpp"
#include "CMPlatform/WinImpl_Graphics.hpp"

namespace CMEngine::Platform::WinImpl
{
	class WinImpl_Platform : public IPlatform
	{
	public:
		WinImpl_Platform() noexcept;
		~WinImpl_Platform() noexcept;

		WinImpl_Platform(const WinImpl_Platform& other) = delete;
		WinImpl_Platform& operator=(const WinImpl_Platform& other) = delete;
	public:
		void Impl_Update() noexcept;

		inline [[nodiscard]] WinImpl::Window& Impl_Window() noexcept { return m_Window; }
		inline [[nodiscard]] WinImpl::Graphics& Impl_Graphics() noexcept { return m_Graphics; }
	private:
		void Impl_Init() noexcept;
		void Impl_Shutdown() noexcept;
	private:
		WinImpl::Window m_Window;
		WinImpl::Graphics m_Graphics;
	};

	CM_DYNAMIC_LOAD IPlatform* WinImpl_Platform_Init();
	CM_DYNAMIC_LOAD void WinImpl_Platform_Shutdown(IPlatform** ppPlatform);

	/* Platform functions. */
	CM_DYNAMIC_LOAD void WinImpl_Platform_Update();
}