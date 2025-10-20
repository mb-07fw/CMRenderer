#include "Editor.hpp"
#include "Component.hpp"

#include <iostream>
#include <chrono>
#include <thread>

namespace CMEngine::Editor
{
	Editor::Editor() noexcept
	{
		std::cout << "Engine Editor!\n";

		ECS::ECS& ecs = m_Core.ECS();
		ECS::Entity camera = ecs.CreateEntity();

		/* Default construct a CameraComponent to avoid copying... */
		ecs.EmplaceComponent<CameraComponent>(camera);

		CameraComponent* pComponent = ecs.GetComponent<CameraComponent>(camera);
		CM_ENGINE_ASSERT(pComponent != nullptr);

		APlatform& platform = m_Core.Platform();

		pComponent->Data.Origin = { 0.0f, 0.0f, -10.0f };
		//pComponent->Data.LookAtPos = { 0.0f, 0.0f, 0.0f }; (set by default)
		pComponent->Data.Aspect = platform.GetWindow().ClientResolution().Aspect();
		pComponent->Data.FovAngle = 45.0f;
		pComponent->Data.NearZ = 0.5f;
		pComponent->Data.FarZ = 1000.0f;

		Scene::SceneManager& sceneManager = m_Core.SceneManager();

		Scene::CameraSystem& cameraSystem = sceneManager.GetCameraSystem();
		cameraSystem.SetMainCamera(camera);

		m_EditorSceneID = sceneManager.NewScene();
		Scene::Scene& scene = sceneManager.RetrieveScene(m_EditorSceneID);

		scene.Graph().AddNode(Scene::Node::NodeType::Camera3D, camera);
	}

	Editor::~Editor() noexcept
	{	
	}

	void Editor::Run() noexcept
	{
		while (m_Core.Platform().IsRunning())
		{
			auto startTime = std::chrono::high_resolution_clock::now();

			m_Core.Update();

			AGraphics& graphics = m_Core.Platform().GetGraphics();

			graphics.StartFrame(Color4::Black());

			m_Core.SceneManager().DisplaySceneGraph();

			graphics.EndFrame();

			/* anti-CPU-combustion... */
			auto endTime = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float, std::milli> delta = std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(endTime - startTime);

			constexpr float TargetFrameTime = 1000.0f / 60.0f;
			float remainingMillis = TargetFrameTime - delta.count();

			auto remainingDuration = std::chrono::duration<float, std::milli>(remainingMillis);

			std::this_thread::sleep_for(remainingDuration);
		}
	}
}