#include "Core/CME_PCH.hpp"
#include "Windows/CME_WNKeyboard.hpp"

#define CM_KEYSTATE_IS_PRESSED_BIT  0x80 // Reserved "pressed" flag by Windows OS.
#define CM_KEYSTATE_WAS_PRESSED_BIT 0x40 // Custom flag to track if a key was pressed previously.
#define CM_KEYSTATE_LOW_ORDER_BYTE  0xFF

namespace CMEngine::WindowsAPI
{
	CMKeyboard::CMKeyboard(CMCommon::CMLoggerWide& logger) noexcept
		: m_Logger(logger), m_KeyStates() // Not sure if std::array initializes to defaults automatically...
	{
		ClearAllState();

		// 0 for layout of current thread.
		m_CurrentInputLocale = GetKeyboardLayout(0);

		if (m_CurrentInputLocale == nullptr)
			m_Logger.LogFatalNL(L"CMKeyboard [()] | Failed to retrieve the current input locale.");

		/* Note: (from GetKeyboardLayout docs) 
		*  https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getkeyboardlayout
		* 
		*   Since the keyboard layout can be dynamically changed,
		*   applications that cache information about the current keyboard layout should
		*   process the WM_INPUTLANGCHANGE message to be informed of changes in the input language.
		*/
	}

	void CMKeyboard::SetPressedVK(BYTE virtualKey) noexcept
	{
		// Set high-order bit (bit 7; pressed flag) and bit 6 (our custom tracking flag) while preserving other bits.
		m_KeyStates[virtualKey] |= CM_KEYSTATE_IS_PRESSED_BIT | CM_KEYSTATE_WAS_PRESSED_BIT;
	}

	void CMKeyboard::SetReleasedVK(BYTE virtualKey) noexcept
	{
		// Clear the high-order bit (bit 7; pressed flag) while preserving other bits.
		m_KeyStates[virtualKey] &= ~CM_KEYSTATE_IS_PRESSED_BIT;
	}

	[[nodiscard]] bool CMKeyboard::IsPressedVK(BYTE virtualKey) noexcept
	{
		// If high order bit is set (1), the key is pressed. Otherwise the key is released.
		return (m_KeyStates[virtualKey] & CM_KEYSTATE_IS_PRESSED_BIT) != 0;
	}

	[[nodiscard]] bool CMKeyboard::IsPressed(wchar_t c) noexcept
	{
		BYTE virtualKey = VKOfWChar(c);
		return IsPressedVK(virtualKey);
	}

	[[nodiscard]] bool CMKeyboard::IsPressed(char c) noexcept
	{
		BYTE virtualKey = VKOfWChar(static_cast<wchar_t>(c));
		return IsPressedVK(virtualKey);
	}

	[[nodiscard]] bool CMKeyboard::IsPressedClear(wchar_t c) noexcept
	{
		BYTE virtualKey = VKOfWChar(c);

		bool isPressed = IsPressedVK(virtualKey);
		ClearStateVK(virtualKey);

		return isPressed;
	}

	[[nodiscard]] bool CMKeyboard::IsPressedClear(char c) noexcept
	{
		BYTE virtualKey = VKOfChar(c);

		bool isPressed = IsPressedVK(virtualKey);
		ClearStateVK(virtualKey);

		return isPressed;
	}

	[[nodiscard]] bool CMKeyboard::IsReleasedVK(BYTE virtualKey) noexcept
	{
		BYTE state = m_KeyStates[virtualKey];

		// If high order bit is not set (0), the key is released. Otherwise the key is pressed.
		// Make sure the key was actually set to pressed via CM_KEYSTATE_WAS_PRESSED_BIT.
		return ((state & CM_KEYSTATE_IS_PRESSED_BIT) == 0) &&
			   ((state & CM_KEYSTATE_WAS_PRESSED_BIT) != 0);
	}

	[[nodiscard]] bool CMKeyboard::IsReleased(wchar_t c) noexcept
	{
		BYTE virtualKey = VKOfWChar(c);
		return IsReleasedVK(virtualKey);
	}

	[[nodiscard]] bool CMKeyboard::IsReleased(char c) noexcept
	{
		BYTE virtualKey = VKOfChar(c);
		return IsReleasedVK(virtualKey);
	}

	[[nodiscard]] bool CMKeyboard::IsReleasedClear(wchar_t c) noexcept
	{
		BYTE virtualKey = VKOfWChar(c);

		bool isReleased = IsReleasedVK(virtualKey);

		if (isReleased)
			ClearStateVK(virtualKey);

		return isReleased;
	}

	[[nodiscard]] bool CMKeyboard::IsReleasedClear(char c) noexcept
	{
		BYTE virtualKey = VKOfChar(c);

		bool isReleased = IsReleasedVK(virtualKey);
		
		if (isReleased)
			ClearStateVK(virtualKey);

		return isReleased;
	}

	void CMKeyboard::ClearStateVK(BYTE virtualKey) noexcept
	{
		m_KeyStates[virtualKey] = 0;
	}

	void CMKeyboard::ClearStateOf(wchar_t c) noexcept
	{
		BYTE virtualKey = VKOfWChar(c);
		ClearStateVK(virtualKey);
	}

	void CMKeyboard::ClearStateOf(char c) noexcept
	{
		BYTE virtualKey = VKOfChar(c);
		ClearStateVK(virtualKey);
	}

	void CMKeyboard::ClearAllState() noexcept
	{
		memset(m_KeyStates.data(), 0, S_TOTAL_VKS);
	}

	[[nodiscard]] wchar_t CMKeyboard::TranslateVK(BYTE virtualKey) noexcept
	{
		BOOL succeeded = GetKeyboardState(m_KeyStates.data());

		m_Logger.LogFatalNLIf(!succeeded, L"CMKeyboard [TranslateVK] | Failed to get keyboard state.");

		wchar_t translatedChar = 0;
		UINT scanCode = MapVirtualKeyW(virtualKey, MAPVK_VK_TO_VSC);

		int result = ToUnicode(virtualKey, scanCode, m_KeyStates.data(), &translatedChar, 1, 0);
		m_Logger.LogFatalNLIf(!result, L"CMKeyboard [TranslateVK] | No translation found.");

		return translatedChar;
	}

	[[nodiscard]] BYTE CMKeyboard::VKOfWChar(wchar_t c) noexcept
	{
		SHORT vkInfo = VkKeyScanExW(c, m_CurrentInputLocale);
		m_Logger.LogFatalNLAppendIf(
			vkInfo == -1,
			L"CMKeyboard [IsKeyPressed] | Failed to get a valid VK for the wchar_t : ",
			c
		);

		BYTE virtualKey = vkInfo & CM_KEYSTATE_LOW_ORDER_BYTE;
		return virtualKey;
	}

	[[nodiscard]] BYTE CMKeyboard::VKOfChar(char c) noexcept
	{
		return VKOfWChar(static_cast<wchar_t>(c));
	}
}