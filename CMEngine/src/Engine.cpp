#include "PCH.hpp"
#include "Engine.hpp"
#include "Platform.hpp"
#include "Asset/AssetManager.hpp"
#include "ECS/ECS.hpp"
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
		LayerStack m_LayerStack;
	};

	/*[[nodiscard]] Asset::AssetID Engine::LoadMesh(std::string_view modelName) noexcept
	{
		std::filesystem::path modelPath = CM_ENGINE_RESOURCES_MODEL_DIRECTORY;
		modelPath /= modelName;

		Asset::AssetID id;
		Asset::Result result = m_AssetManager.LoadMesh(modelPath, id);

		return id;
	}*/

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

	EngineImpl::EngineImpl() noexcept
	{
		m_LayerStack.AddLayer(std::make_shared<EditorLayer>(m_Platform, m_AssetManager, m_ECS));
	}

	void EngineImpl::Update() noexcept
	{
		m_Platform.Update();
		m_LayerStack.Update();
	}
}