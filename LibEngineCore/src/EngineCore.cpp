#include "PCH.hpp"
#include "EngineCore.hpp"

namespace CMEngine
{
	EngineCore::EngineCore() noexcept
		: m_Renderer(m_ECS, m_Platform.GetGraphics(), m_AssetManager),
		  m_SceneManager(m_ECS)
	{
	}

	EngineCore::~EngineCore() noexcept
	{
	}

	void EngineCore::Update() noexcept
	{
		m_Platform.Update();
	}
}