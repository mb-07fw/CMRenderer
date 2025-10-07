#pragma once

#include "Layer.hpp"

#include <cstdint>
#include <memory>
#include <vector>
#include <type_traits>

namespace CMEngine
{
	class LayerStack
	{
	public:
		LayerStack() = default;
		~LayerStack() = default;
	public:
		template <typename Ty>
			requires std::is_base_of_v<ILayer, Ty>
		inline std::weak_ptr<Ty> AddLayer(const std::shared_ptr<Ty> pLayer) noexcept;

		template <typename Ty>
			requires std::is_base_of_v<ILayer, Ty>
		inline bool RemoveLayer() noexcept;

		template <typename Ty>
			requires std::is_base_of_v<ILayer, Ty>
		inline [[nodiscard]] std::weak_ptr<Ty> GetLayer() noexcept;

		void Update() noexcept;
	private:
		std::vector<std::shared_ptr<ILayer>> m_Layers;
	};

	template <typename Ty>
		requires std::is_base_of_v<ILayer, Ty>
	inline std::weak_ptr<Ty> LayerStack::AddLayer(const std::shared_ptr<Ty> pLayer) noexcept
	{
		m_Layers.emplace_back(pLayer);

		pLayer->OnAttach();

		return std::weak_ptr<Ty>(pLayer);
	}

	template <typename Ty>
		requires std::is_base_of_v<ILayer, Ty>
	inline bool LayerStack::RemoveLayer() noexcept
	{
		for (size_t i = 0; i < m_Layers.size(); ++i)
			if (auto pDerived = std::dynamic_pointer_cast<Ty>(m_Layers[i]))
			{
				pDerived->OnDetatch();
				m_Layers.erase(m_Layers.begin() + i);
				return true;
			}

		return false;
	}

	template <typename Ty>
		requires std::is_base_of_v<ILayer, Ty>
	inline [[nodiscard]] std::weak_ptr<Ty> LayerStack::GetLayer() noexcept
	{
		for (const std::shared_ptr<ILayer>& pLayer : m_Layers)
			if (auto pDerived = std::dynamic_pointer_cast<Ty>(pLayer))
				return pDerived;
	}
}