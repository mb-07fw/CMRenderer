#include "Core/CME_PCH.hpp"
#include "Core/CME_LayerStack.hpp"

namespace CMEngine
{
	CMLayerStack::CMLayerStack(CMCommon::CMLoggerWide& logger) noexcept
		: m_Logger(logger)
	{
	}

	void CMLayerStack::Update(float deltaTime) noexcept
	{
		for (auto it = m_Layers.begin(); it != m_Layers.end(); ++it)
			(*it)->OnUpdate(deltaTime);
	}

	void CMLayerStack::Clear() noexcept
	{
		for (auto rit = m_Layers.rbegin(); rit != m_Layers.rend(); ++rit)
			(*rit)->OnDetach();

		m_Layers.clear();
	}

	[[nodiscard]] std::shared_ptr<ICMLayer> CMLayerStack::Retrieve(CMLayerHandle handle) noexcept
	{
		if (handle.IsInvalid() || handle.Index >= m_Layers.size())
			return std::shared_ptr<ICMLayer>(nullptr);

		return m_Layers[handle.Index];
	}
}