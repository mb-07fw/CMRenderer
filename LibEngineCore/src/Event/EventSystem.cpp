#include "PCH.hpp"
#include "EventSystem.hpp"
#include "Macros.hpp"

namespace CMEngine::Event
{
	[[nodiscard]] ObserverID EventSystem::Subscribe(EventType type, const std::function<void(const IEvent&)>& onEventFunc) noexcept
	{
		CM_ENGINE_ASSERT(type != EventType::Invalid);
		CM_ENGINE_ASSERT(type != EventType::Total_Events);

		std::vector<std::function<void(const IEvent&)>>& callbacks = m_Callbacks[(size_t)type];
		ObserverID id(type, callbacks.size());

		callbacks.emplace_back(onEventFunc);
		return id;
	}

	void EventSystem::Unsubscribe(const ObserverID& id) noexcept
	{
		std::vector<std::function<void(const IEvent&)>>& callbacks = m_Callbacks[(size_t)id.ObservedType];

		if (id.Index >= callbacks.size())
			return;

		callbacks.erase(callbacks.begin() + id.Index);
	}
}