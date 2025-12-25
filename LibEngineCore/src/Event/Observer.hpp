#pragma once

#include <cstdint>

#include "Event.hpp"

namespace CMEngine::Event
{
	struct ObserverID
	{
		EventType ObservedType = EventType::Invalid;
		size_t Index = 0;

		inline ObserverID(EventType type, size_t index) noexcept
			: ObservedType(type),
			  Index(index)
		{
		}

		ObserverID() = default;
		~ObserverID() = default;
	};
}