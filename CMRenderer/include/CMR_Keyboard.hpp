#pragma once

#include "CMC_Logger.hpp"

#include <minwindef.h>
#include <array>

namespace CMRenderer
{
	// NOTE : char: ASCII hotkey support (e.g., 'A', 'Z').
	// NOTE : wchar_t: Unicode key translation for localized input.
	class CMKeyboard
	{
	public:
		CMKeyboard(CMCommon::CMLoggerWide& cmLoggerRef) noexcept;
		~CMKeyboard() = default;
	public:
		void SetPressedVK(BYTE virtualKey) noexcept;
		void SetReleasedVK(BYTE virtualKey) noexcept;

		[[nodiscard]] bool IsPressedVK(BYTE virtualKey) noexcept;
		[[nodiscard]] bool IsPressed(wchar_t c) noexcept;
		[[nodiscard]] bool IsPressed(char c) noexcept;
		[[nodiscard]] bool IsPressedClear(wchar_t c) noexcept;
		[[nodiscard]] bool IsPressedClear(char c) noexcept;

		[[nodiscard]] bool IsReleasedVK(BYTE virtualKey) noexcept;
		[[nodiscard]] bool IsReleased(wchar_t c) noexcept;
		[[nodiscard]] bool IsReleased(char c) noexcept;
		[[nodiscard]] bool IsReleasedClear(wchar_t c) noexcept;
		[[nodiscard]] bool IsReleasedClear(char c) noexcept;

		void ClearStateVK(BYTE virtualKey) noexcept;
		void ClearStateOf(wchar_t c) noexcept;
		void ClearStateOf(char c) noexcept;

		void ClearAllState() noexcept;

		[[nodiscard]] wchar_t TranslateVK(BYTE virtualKey) noexcept;
		[[nodiscard]] BYTE VKOfWChar(wchar_t c) noexcept;
		[[nodiscard]] BYTE VKOfChar(char c) noexcept;
	private:
		static constexpr USHORT S_TOTAL_VKS = 256;
		std::array<BYTE, S_TOTAL_VKS> m_KeyStates;
		HKL m_CurrentInputLocale = nullptr;
		CMCommon::CMLoggerWide& m_CMLoggerRef;
	};
}