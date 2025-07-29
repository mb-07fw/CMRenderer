#include "Core/PCH.hpp"
#include "Core/LayerStack.hpp"

namespace CMEngine::Core
{
	LayerStack::LayerStack(Common::LoggerWide& logger) noexcept
		: m_Logger(logger)
	{
	}

	void LayerStack::Update(float deltaTime) noexcept
	{
		for (auto it = m_Layers.begin(); it != m_Layers.end(); ++it)
			(*it)->OnUpdate(deltaTime);
	}

	void LayerStack::Clear() noexcept
	{
		for (auto rit = m_Layers.rbegin(); rit != m_Layers.rend(); ++rit)
			(*rit)->OnDetach();

		m_Layers.clear();
	}

	[[nodiscard]] std::shared_ptr<ILayer> LayerStack::Retrieve(LayerHandle handle) noexcept
	{
		if (handle.IsInvalid() || handle.Index >= m_Layers.size())
			return std::shared_ptr<ILayer>(nullptr);

		return m_Layers[handle.Index];
	}
}