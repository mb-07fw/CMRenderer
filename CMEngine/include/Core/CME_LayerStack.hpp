#pragma once

#include "CME_Layer.hpp"
#include "CMC_Logger.hpp"

#include <vector>
#include <memory>

namespace CMEngine
{
	struct CMLayerHandle
	{
		using LayerIndex = size_t;
		static constexpr LayerIndex S_INVALID_INDEX = static_cast<LayerIndex>(-1);

		LayerIndex Index = S_INVALID_INDEX;

		inline static constexpr [[nodiscard]] bool IsInvalidHandle(CMLayerHandle handle) noexcept
		{
			return handle.Index == S_INVALID_INDEX;
		}

		inline static constexpr [[nodiscard]] bool IsValidHandle(CMLayerHandle handle) noexcept
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

	class CMLayerStack
	{
	public:
		CMLayerStack(CMCommon::CMLoggerWide& logger) noexcept;
		~CMLayerStack() = default;
	public:
		void Update(float deltaTime) noexcept;

		template <typename Ty, typename... Args>
			requires std::is_base_of_v<ICMLayer, Ty>
		[[nodiscard]] CMLayerHandle EmplaceBack(Args&&... args) noexcept;

		[[nodiscard]] std::shared_ptr<ICMLayer> Retrieve(CMLayerHandle handle) noexcept;

		template <typename Ty>
			requires std::is_base_of_v<ICMLayer, Ty>
		[[nodiscard]] std::shared_ptr<Ty> RetrieveAs(CMLayerHandle handle) noexcept;

		void Clear() noexcept;
	private:
		std::vector<std::shared_ptr<ICMLayer>> m_Layers;
		CMCommon::CMLoggerWide& m_Logger;
	};

	template <typename Ty, typename... Args>
		requires std::is_base_of_v<ICMLayer, Ty>
	[[nodiscard]] CMLayerHandle CMLayerStack::EmplaceBack(Args&&... args) noexcept
	{
		size_t nextIndex = m_Layers.size();

		m_Layers.emplace_back(std::make_shared<Ty>(std::forward<Args>(args)...));
		m_Layers.back()->OnAttach();

		return CMLayerHandle{ nextIndex };
	}

	template <typename Ty>
		requires std::is_base_of_v<ICMLayer, Ty>
	[[nodiscard]] std::shared_ptr<Ty> CMLayerStack::RetrieveAs(CMLayerHandle handle) noexcept
	{
		return std::dynamic_pointer_cast<Ty>(Retrieve(handle));
	}
}