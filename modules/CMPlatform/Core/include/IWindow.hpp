#pragma once

#include "Core_Export.hpp"
#include "Types.hpp"

#include <functional>
#include <type_traits>

namespace CMPlatform
{
	using WindowCallbackSignatureOnResize = void (*)(Float2, void* pUserData);

	template <typename Callback>
	concept WindowActionCallback = std::is_same_v<Callback, WindowCallbackSignatureOnResize>;

	template <typename CallbackSignature>
		requires WindowActionCallback<CallbackSignature>
	struct CM_PLATFORM_API WindowCallback
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

	class CM_PLATFORM_API IWindow
	{
	public:
		IWindow() noexcept;
		virtual ~IWindow() = default;

		IWindow(const IWindow& other) = delete;
		IWindow& operator=(const IWindow& other) = delete;
	public:
		virtual void Update() noexcept = 0;

		virtual [[nodiscard]] bool IsRunning() const noexcept = 0;
		virtual [[nodiscard]] bool ShouldClose() const noexcept = 0;
	};
}