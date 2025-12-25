#pragma once

#include <array>
#include <vector>
#include <functional>
#include <type_traits>

#include "Event.hpp"
#include "Observer.hpp"

namespace CMEngine::Event
{
	class EventSystem
	{
	public:
		EventSystem() = default;
		~EventSystem() = default;
	public:
		template <typename Ty>
			requires std::is_base_of_v<IEvent, Ty>
		inline [[nodiscard]] ObserverID Subscribe(
			const std::function<void(const IEvent&)>& onEventFunc
		) noexcept;

		[[nodiscard]] ObserverID Subscribe(
			EventType type, 
			const std::function<void(const IEvent&)>& onEventFunc
		) noexcept;

		void Unsubscribe(const ObserverID& id) noexcept;

		template <typename Ty>
			requires std::is_base_of_v<IEvent, Ty>
		inline void Dispatch(const Ty& event) noexcept;
	private:
		std::array<std::vector<std::function<void(const IEvent&)>>, (size_t)EventType::Total_Events> m_Callbacks;
	};

	template <typename Ty>
		requires std::is_base_of_v<IEvent, Ty>
	inline [[nodiscard]] ObserverID EventSystem::Subscribe(
		const std::function<void(const IEvent&)>& onEventFunc
	) noexcept
	{
		constexpr EventType EType = Ty::S_Corresponding_Type;

		static_assert(
			EType != EventType::Invalid && EType != EventType::Total_Events,
			"Event type Ty has an illegal corresponding type."
		);

		std::vector<std::function<void(const IEvent&)>>& callbacks = m_Callbacks[(size_t)EType];
		ObserverID id(EType, callbacks.size());

		callbacks.emplace_back(onEventFunc);
		return id;
	}

	template <typename Ty>
		requires std::is_base_of_v<IEvent, Ty>
	inline void EventSystem::Dispatch(const Ty& event) noexcept
	{
		constexpr EventType EType = Ty::S_Corresponding_Type;

		static_assert(
			EType != EventType::Invalid && EType != EventType::Total_Events,
			"Event type Ty has an illegal corresponding type."
		);

		for (const auto& callback : m_Callbacks[(size_t)EType])
			callback(event);
	}
}