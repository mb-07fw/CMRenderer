#pragma once

#include "Export.hpp"
#include "Types.hpp"

#include <functional>
#include <type_traits>

namespace CMEngine
{
	using WindowCallbackSignatureOnResize = void (*)(Float2, void* pUserData);

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

	template struct CM_ENGINE_API WindowCallback<WindowCallbackSignatureOnResize>;
	using WindowCallbackOnResize = WindowCallback<WindowCallbackSignatureOnResize>;

	class CM_ENGINE_API IWindow
	{
	public:
		IWindow() = default;
		virtual ~IWindow() = default;

		IWindow(const IWindow& other) = delete;
		IWindow& operator=(const IWindow& other) = delete;
	public:
		virtual void Update() noexcept = 0;

		virtual [[nodiscard]] bool IsRunning() const noexcept = 0;
		virtual [[nodiscard]] bool ShouldClose() const noexcept = 0;
	};
}