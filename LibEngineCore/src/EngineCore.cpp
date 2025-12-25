#include "PCH.hpp"
#include "EngineCore.hpp"

namespace CMEngine
{
	EngineCore::EngineCore() noexcept
		: m_Platform(m_EventSystem), 
		  m_Renderer(m_ECS, m_Platform.GetGraphics(), m_AssetManager),
		  m_SceneManager(m_ECS, m_Platform.GetWindow())
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