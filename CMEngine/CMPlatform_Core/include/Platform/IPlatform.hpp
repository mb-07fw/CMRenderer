#pragma once

#include "Platform/IWindow.hpp"
#include "Platform/IGraphics.hpp"

#include <memory>

namespace CMEngine::Platform
{
	class IPlatform;

	struct PlatformFuncTable
	{
		using VoidFunc = void (*)();
		
		VoidFunc Update = nullptr;

		inline PlatformFuncTable(
			VoidFunc UpdateFunc
		) noexcept
			: Update(UpdateFunc)
		{
		}
	};

	class IPlatform
	{
	public:
		void Update() noexcept;

		inline [[nodiscard]] IWindow* Window() noexcept { return mP_Window; }
		inline [[nodiscard]] IGraphics* Graphics() noexcept { return mP_Graphics; }
	protected:
		IPlatform(
			IWindow* pWindow,
			IGraphics* pGraphics,
			const PlatformFuncTable& funcTable
		) noexcept;

		virtual ~IPlatform() = default;

		IPlatform(const IPlatform& other) = delete;
		IPlatform& operator=(const IPlatform& other) = delete;
	private:
		void VerifyFuncTable() const noexcept;
	private:
		IWindow* mP_Window;
		IGraphics* mP_Graphics;
		PlatformFuncTable m_FuncTable;
	};
}