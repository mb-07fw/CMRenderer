#pragma once

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

	template struct WindowCallback<WindowCallbackSignatureOnResize>;
	using WindowCallbackOnResize = WindowCallback<WindowCallbackSignatureOnResize>;

	enum class KeycodeType : uint8_t
	{
		Unknown,
		F1,
		Return,
		Back, // Backspace...
		Alt,
		Shift,
		Ctrl,
		Space,
		A, B, C, D, E, F, G, H, I, J, K, L, M,
		N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

		Total_Keys = Z,
		Total_Values = Z + 1
	};

	class IWindow
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