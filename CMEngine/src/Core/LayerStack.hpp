#pragma once

#include "Core/Layer.hpp"
#include "Common/Logger.hpp"

#include <vector>
#include <memory>

namespace CMEngine::Core
{
	struct LayerHandle
	{
		using LayerIndex = size_t;
		static constexpr LayerIndex S_INVALID_INDEX = static_cast<LayerIndex>(-1);

		LayerIndex Index = S_INVALID_INDEX;

		inline static constexpr [[nodiscard]] bool IsInvalidHandle(LayerHandle handle) noexcept
		{
			return handle.Index == S_INVALID_INDEX;
		}

		inline static constexpr [[nodiscard]] bool IsValidHandle(LayerHandle handle) noexcept
		{
			return handle.Index != S_INVALID_INDEX;
		}

		inline constexpr [[nodiscard]] bool IsInvalid() const noexcept
		{
			return IsInvalidHandle(*this);
		}

		inline constexpr [[nodiscard]] bool IsValid() const noexcept
		{
			return IsValidHandle(*this);
		}
	};

	class LayerStack
	{
	public:
		LayerStack(Common::LoggerWide& logger) noexcept;
		~LayerStack() = default;
	public:
		void Update(float deltaTime) noexcept;

		template <typename Ty, typename... Args>
			requires std::is_base_of_v<ILayer, Ty>
		[[nodiscard]] LayerHandle EmplaceBack(Args&&... args) noexcept;

		[[nodiscard]] std::shared_ptr<ILayer> Retrieve(LayerHandle handle) noexcept;

		template <typename Ty>
			requires std::is_base_of_v<ILayer, Ty>
		[[nodiscard]] std::shared_ptr<Ty> RetrieveAs(LayerHandle handle) noexcept;

		void Clear() noexcept;
	private:
		std::vector<std::shared_ptr<ILayer>> m_Layers;
		Common::LoggerWide& m_Logger;
	};

	template <typename Ty, typename... Args>
		requires std::is_base_of_v<ILayer, Ty>
	[[nodiscard]] LayerHandle LayerStack::EmplaceBack(Args&&... args) noexcept
	{
		size_t nextIndex = m_Layers.size();

		m_Layers.emplace_back(std::make_shared<Ty>(std::forward<Args>(args)...));
		m_Layers.back()->OnAttach();

		return LayerHandle{ nextIndex };
	}

	template <typename Ty>
		requires std::is_base_of_v<ILayer, Ty>
	[[nodiscard]] std::shared_ptr<Ty> LayerStack::RetrieveAs(LayerHandle handle) noexcept
	{
		return std::dynamic_pointer_cast<Ty>(Retrieve(handle));
	}
}