#pragma once

#include <cstdint>

#include "Platform/Core/IWindow.hpp"

namespace CMEngine::Event
{
	enum class EventType : int8_t
	{
		Invalid = -1,

		Window_Resize = 0,

		Input_Key_Pressed,
		Input_Key_Released,

		Total_Events
	};

	struct IEvent
	{
	public:
		IEvent() = default;
		virtual ~IEvent() = default;
	};

	struct IKeyEvent : public IEvent
	{
	public:
		IKeyEvent() = default;
		virtual ~IKeyEvent() = default;
	};

	template <EventType EType>
	struct EventBasic
	{
	public:
		static constexpr EventType S_Corresponding_Type = EType;

		EventBasic() = default;
		~EventBasic() = default;
	};

	struct KeyPressed : public IKeyEvent,
		public EventBasic<EventType::Input_Key_Pressed>
	{
		KeyPressed(KeycodeType type) noexcept;
		KeyPressed() = default;
		~KeyPressed() = default;

		KeycodeType Keycode = KeycodeType::Unknown;
	};

	struct KeyReleased : public IKeyEvent,
		public EventBasic<EventType::Input_Key_Released>
	{
		KeyReleased(KeycodeType type) noexcept;
		KeyReleased() = default;
		~KeyReleased() = default;

		KeycodeType Keycode = KeycodeType::Unknown;
	};
}