#include "Editor.hpp"
#include "Component.hpp"

#include <iostream>
#include <chrono>
#include <thread>

namespace CMEngine::Editor
{
	Editor::Editor() noexcept
	{
		ECS::ECS& ecs = m_Core.ECS();

		ECS::Entity cameraEntity = ecs.CreateEntity();
		ECS::Entity gameObj1 = ecs.CreateEntity();
		ECS::Entity gameObj2 = ecs.CreateEntity();

		Asset::AssetManager& assetManager = m_Core.AssetManager();

		constexpr std::string_view MeshName = ENGINE_EDITOR_RESOURCES_MODEL_DIRECTORY "/cube.gltf";

		Asset::AssetID modelID;
		Asset::Result result = assetManager.LoadModel(MeshName, modelID);
		CM_ENGINE_ASSERT(result.Succeeded());

		ConstView<Asset::Model> model;
		assetManager.GetModel(modelID, model);
		CM_ENGINE_ASSERT(model.NonNull());

		Asset::AssetID meshID = model->Meshes.at(0);
		Asset::AssetID materialID = model->Materials.at(0);

		/* Default construct components to avoid extra copying... */
		ecs.EmplaceComponent<CameraComponent>(cameraEntity);
		ecs.EmplaceComponent<TransformComponent>(gameObj1);
		ecs.EmplaceComponent<TransformComponent>(gameObj2);

		TransformComponent& gObj1Transform = ecs.GetComponent<TransformComponent>(gameObj1);
		TransformComponent& gObj2Transform = ecs.GetComponent<TransformComponent>(gameObj2);

		gObj1Transform.Transform.Translation.x = -1.0f;
		gObj2Transform.Transform.Translation.x = 1.0f;

		gObj1Transform.Transform.Scaling.x = 0.5f;
		gObj1Transform.Transform.Scaling.y = 0.5f;
		gObj1Transform.Transform.Scaling.z = 0.5f;

		gObj2Transform.Transform.Scaling.x = 0.5f;
		gObj2Transform.Transform.Scaling.y = 0.5f;
		gObj2Transform.Transform.Scaling.z = 0.5f;

		gObj1Transform.CreateModelMatrix();
		gObj2Transform.CreateModelMatrix();

		ecs.EmplaceComponent<MeshComponent>(gameObj1, meshID);
		ecs.EmplaceComponent<MeshComponent>(gameObj2, meshID);
		ecs.EmplaceComponent<MaterialComponent>(gameObj1, materialID);
		ecs.EmplaceComponent<MaterialComponent>(gameObj2, materialID);

		m_Core.Renderer().GetBatchRenderer().SubmitMesh(gameObj1);

		Scene::SceneManager& sceneManager = m_Core.SceneManager();

		m_EditorSceneID = sceneManager.NewScene();
		Scene::Scene& scene = sceneManager.RetrieveScene(m_EditorSceneID);
		Scene::SceneGraph& sceneGraph = scene.Graph();

		sceneGraph.AddNode(Scene::Node::NodeType::Camera3D, cameraEntity);
		sceneGraph.AddNode(Scene::Node::NodeType::GameObject, gameObj1);
		sceneGraph.AddNode(Scene::Node::NodeType::GameObject, gameObj2);

		auto& camera = ecs.GetComponent<CameraComponent>(cameraEntity);

		APlatform& platform = m_Core.Platform();

		camera.Data.Origin = { 0.0f, 0.0f, -10.0f };
		//camera.Data.LookAtPos = { 0.0f, 0.0f, 0.0f }; (set by default)
		camera.Data.Aspect = platform.GetWindow().ClientResolution().Aspect();
		camera.Data.FovAngle = 45.0f;
		camera.Data.NearZ = 0.5f;
		camera.Data.FarZ = 1000.0f;
		camera.ProjDirty = true;
		camera.ViewDirty = true;

		Scene::CameraSystem& cameraSystem = sceneManager.GetCameraSystem();
		cameraSystem.SetMainCamera(cameraEntity);
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
			Renderer::Renderer& renderer = m_Core.Renderer();
			ECS::ECS& ecs = m_Core.ECS();
			Scene::SceneManager& sceneManager = m_Core.SceneManager();
			Asset::AssetManager& assetManager = m_Core.AssetManager();
			Scene::Scene& scene = sceneManager.RetrieveScene(m_EditorSceneID);

			renderer.StartFrame(Color4::Black());

			Renderer::BatchRenderer& batchRenderer = renderer.GetBatchRenderer();

			batchRenderer.BeginBatch();

			for (const auto& node : scene.Graph().Root().Nodes)
				if (node.Type == Scene::Node::NodeType::GameObject)
				{
					auto mesh = ecs.GetComponent<MeshComponent>(node.Entity);
					auto material = ecs.GetComponent<MaterialComponent>(node.Entity);

					batchRenderer.SubmitInstance(mesh.ID, material.ID, node.Entity);
				}

			batchRenderer.EndBatch();

			Scene::CameraSystem& cameraSystem = sceneManager.GetCameraSystem();
			View<CameraComponent> mainCamera = cameraSystem.GetMainCamera();

			if (mainCamera.NonNull() && mainCamera->Dirty())
			{
				mainCamera->UpdateMatrices();
				renderer.SetCamera(*mainCamera);
			}

			renderer.Flush();

			sceneManager.DisplaySceneGraph();
			
			renderer.EndFrame();

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