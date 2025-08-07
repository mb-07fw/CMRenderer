#pragma once

#include "CMPlatform/Types.hpp"

#include <functional>
#include <type_traits>

namespace CMEngine::Platform
{
	struct ScreenResolution
	{
		Float2 Res;
	};

	struct WindowFuncTable
	{
		using VoidFunc = void(*)();
		using BoolFunc = bool(*)();
		using ResolutionFunc = ScreenResolution(*)();

		VoidFunc Update = nullptr;
		BoolFunc IsRunning = nullptr;
		BoolFunc ShouldClose = nullptr;
		ResolutionFunc Resolution = nullptr;

		inline WindowFuncTable(
			VoidFunc updateFunc,
			BoolFunc isRunningFunc,
			BoolFunc shouldCloseFunc,
			ResolutionFunc resolutionFunc
		) noexcept
			: Update(updateFunc),
			  IsRunning(isRunningFunc),
			  ShouldClose(shouldCloseFunc),
			  Resolution(resolutionFunc)
		{
		}
	};

	using WindowCallbackSignatureOnResize = void (*)(ScreenResolution, void* pUserData);

	template <typename Callback>
	concept WindowActionCallback = std::is_same_v<Callback, WindowCallbackSignatureOnResize>;

	template <typename CallbackSignature>
		requires WindowActionCallback<CallbackSignature>
	struct WindowCallback
	{
		CallbackSignature pCallback = nullptr;
		void* pUserData = nullptr;

		inline WindowCallback(CallbackSignature pCallback, void* pUserData) noexcept
			: pCallback(pCallback),
			  pUserData(pUserData)
		{
		}

		WindowCallback() = default;
	};

	using WindowCallbackOnResize = WindowCallback<WindowCallbackSignatureOnResize>;

	class IWindow
	{
	public:
		IWindow(const WindowFuncTable& funcTable) noexcept;
		virtual ~IWindow() = default;

		IWindow(const IWindow& other) = delete;
		IWindow& operator=(const IWindow& other) = delete;
	public:
		inline void Update() noexcept { return m_FuncTable.Update(); }

		inline [[nodiscard]] bool IsRunning() const noexcept { return m_FuncTable.IsRunning(); }
		inline [[nodiscard]] bool ShouldClose() const noexcept { return m_FuncTable.ShouldClose(); }
	private:
		WindowFuncTable m_FuncTable;
	};
}