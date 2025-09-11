#pragma once

#include "Platform/Core/IWindow.hpp"
#include "Platform/Core/IGraphics.hpp"

#include "Types.hpp"

namespace CMEngine
{
	class CM_ENGINE_API IPlatform
	{
	public:
		IPlatform() = default;

		virtual ~IPlatform() = default;

		IPlatform(const IPlatform& other) = delete;
		IPlatform& operator=(const IPlatform& other) = delete;

		virtual [[nodiscard]] bool IsRunning() const noexcept = 0;
		virtual bool Update() noexcept = 0;

		inline [[nodiscard]] IWindow* GetWindow() noexcept { return mP_Window; }
		inline [[nodiscard]] IGraphics* GetGraphics() noexcept { return mP_Graphics; }
	protected:
		void SetInterfaces(IWindow& window, IGraphics& graphics) noexcept;
	private:
		IWindow* mP_Window = nullptr;
		IGraphics* mP_Graphics = nullptr;
	};

	/* Explicitly export these template instantiations. */
	template class CM_ENGINE_API SharedPtr<IPlatform>;

	/* Each derived platform MUST implement this free function, or an unresolved linker error will occur. */
	[[nodiscard]] SharedPtr<IPlatform> MakePlatform() noexcept;
}