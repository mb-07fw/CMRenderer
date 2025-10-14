#include "PCH.hpp"
#include "Engine.hpp"
#include "Platform.hpp"
#include "Asset/AssetManager.hpp"
#include "ECS/ECS.hpp"
#include "Scene/SceneManager.hpp"
#include "LayerStack.hpp"

namespace CMEngine
{
	class EngineImpl
	{
	public:
		EngineImpl() noexcept;
		~EngineImpl() = default;

		EngineImpl(const Engine&) = delete;
		EngineImpl(Engine&&) = delete;
		EngineImpl& operator=(const Engine&) = delete;
		EngineImpl& operator=(Engine&&) = delete;
	public:
		void Update() noexcept;

		inline [[nodiscard]] bool IsRunning() const noexcept { return m_Platform.IsRunning(); }

		inline [[nodiscard]] APlatform& GetPlatform() noexcept { return m_Platform; }
		inline [[nodiscard]] Asset::AssetManager& GetAssetManager() noexcept { return m_AssetManager; }
		inline [[nodiscard]] ECS::ECS& GetECS() noexcept { return m_ECS; }
		inline [[nodiscard]] LayerStack& GetLayerStack() noexcept { return m_LayerStack; }
	private:
		APlatform m_Platform;
		Asset::AssetManager m_AssetManager;
		ECS::ECS m_ECS;
		Scene::SceneManager m_SceneManager;
		LayerStack m_LayerStack;
	};

	EngineImpl::EngineImpl() noexcept
		: m_SceneManager(m_ECS)
	{
		m_LayerStack.AddLayer(std::make_shared<EditorLayer>(m_Platform, m_AssetManager, m_SceneManager, m_ECS));
	}

	void EngineImpl::Update() noexcept
	{
		m_Platform.Update();
		m_LayerStack.Update();
	}

	Engine::Engine() noexcept
		: mP_Impl(new EngineImpl)
	{
	}

	Engine::~Engine() noexcept
	{
		delete mP_Impl;
	}

	void Engine::Run() noexcept
	{
		double deltaTime = 0.0f;
		while (mP_Impl->IsRunning())
		{
			auto startTime = std::chrono::high_resolution_clock::now();

			mP_Impl->Update();

			auto endTime = std::chrono::high_resolution_clock::now();

			deltaTime = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count());

			constexpr double TargetFrameTime = 1000.0f / 60.0f;

			std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(TargetFrameTime - deltaTime));
		}
	}
}