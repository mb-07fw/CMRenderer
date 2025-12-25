#include "PCH.hpp"
#include "Event.hpp"

namespace CMEngine::Event
{
	KeyPressed::KeyPressed(KeycodeType type) noexcept
		: Keycode(type)
	{
	}

	KeyReleased::KeyReleased(KeycodeType type) noexcept
		: Keycode(type)
	{
	}
}