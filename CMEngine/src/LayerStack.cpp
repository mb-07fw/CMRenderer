#include "PCH.hpp"
#include "LayerStack.hpp"

namespace CMEngine
{
	void LayerStack::Update() noexcept
	{
		for (const std::shared_ptr<ILayer> pLayer : m_Layers)
			pLayer->OnUpdate();
	}
}