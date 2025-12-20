#include "Editor.hpp"
#include "Component.hpp"

#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>

struct A
{
	A(int x)
		: X(x)
	{
		std::cout << "Constructed A!\n";
	}

	A(const A& other) noexcept
		: X(other.X)
	{
		std::cout << "Copy constructed A!\n";
	}

	A(A&& other) noexcept
		: X(other.X)
	{
		std::cout << "Move constructed A!\n";
	}


	A& operator=(const A& other) noexcept
	{
		X = other.X;
		std::cout << "Copy assigned A!\n";
		return *this;
	}

	A& operator=(A&& other) noexcept
	{
		X = other.X;
		std::cout << "Move assigned A!\n";
		return *this;
	}

	int X = 0;
};

struct B
{
	B(float x)
		: X(x)
	{
		std::cout << "Constructed B!\n";
	}

	B(const B& other) noexcept
		: X(other.X)
	{
		std::cout << "Copy constructed B!\n";
	}

	B(B&& other) noexcept
		: X(other.X)
	{
		std::cout << "Move constructed B!\n";
	}

	B& operator=(const B& other) noexcept
	{
		X = other.X;
		std::cout << "Copy assigned B!\n";
		return *this;
	}

	B& operator=(B&& other) noexcept
	{
		X = other.X;
		std::cout << "Move assigned B!\n";
		return *this;
	}

	float X = 0.0f;
};

struct TestA
{
	TestA(const A& a) noexcept
		: a(a)
	{
	}

	TestA(const TestA& other) noexcept
		: a(other.a)
	{
		std::cout << "Copy constructed TestA!\n";
	}

	TestA(TestA&& other) noexcept
		: a(std::move(other.a))
	{
		std::cout << "Move constructed TestA!\n";
	}


	TestA& operator=(const TestA& other) noexcept
	{
		a = other.a;
		std::cout << "Copy assigned TestA!\n";
	}

	TestA& operator=(TestA&& other) noexcept
	{
		a = std::move(other.a);
		std::cout << "Move assigned TestA!\n";
	}

	A a;
};

struct TestB
{
	TestB(const B& b) noexcept
		: b(b)
	{
	}

	TestB(const TestB& other) noexcept
		: b(other.b)
	{
		std::cout << "Copy constructed TestB!\n";
	}

	TestB(TestB&& other) noexcept
		: b(std::move(other.b))
	{
		std::cout << "Move constructed TestB!\n";
	}


	TestB& operator=(const TestB& other) noexcept
	{
		b = other.b;
		std::cout << "Copy assigned TestB!\n";
	}

	TestB& operator=(TestB&& other) noexcept
	{
		b = std::move(other.b);
		std::cout << "Move assigned TestB!\n";
	}

	B b;
};

namespace CMEngine::Editor
{
	Editor::Editor() noexcept
	{
		ECS::ECS& ecs = m_Core.ECS();

		/* Archetype testing shenanigans... */
		View<ECS::Archetype<A, B>> archetype = ecs.CreateArchetype<A, B>();
		CM_ENGINE_ASSERT(archetype.NonNull());
		
		ECS::Entity test = ecs.CreateEntity();

		ecs.EmplaceRow(
			test,
			archetype.Ref(),
			MakeParams<A>(2399),
			MakeParams<B>(43.0f)
		);

		auto getTest = ecs.GetArchetype<A, B>();
		auto newArchetype = ecs.ArchetypeAdd<float>(test, archetype.Ref(), 6454.0f);

		auto getTestTwo = ecs.GetArchetype<A, B, float>();
		auto getTestThree = ecs.GetArchetype<int, float, short>();

		bool destroyed = ecs.DestroyEntity(test);
		destroyed = ecs.DestroyArchetype(getTest);
		destroyed = ecs.DestroyArchetype(getTestTwo);
		destroyed = ecs.DestroyArchetype(getTestThree);

		ECS::Entity cameraEntity = ecs.CreateEntity();
		ECS::Entity gameObj1 = ecs.CreateEntity();
		ECS::Entity gameObj2 = ecs.CreateEntity();
			
		Asset::AssetManager& assetManager = m_Core.AssetManager();

		/* TODO: Fix weird DeadlyImportError exception... */
		constexpr std::string_view MeshName = ENGINE_EDITOR_RESOURCES_MODEL_DIRECTORY "/test_cube.glb";

		Asset::AssetID modelID;
		Asset::Result result = assetManager.LoadModel(MeshName, modelID);

		CM_ENGINE_ASSERT(result.Succeeded());

		ConstView<Asset::Model> model;
		assetManager.GetModel(modelID, model);

		CM_ENGINE_ASSERT(model.NonNull());

		Asset::AssetID meshID = model->Meshes.at(0);
		Asset::AssetID materialID = model->Materials.at(0);

		ConstView<Asset::Mesh> meshAsset;
		assetManager.GetMesh(meshID, meshAsset);

		CM_ENGINE_ASSERT(meshAsset.NonNull());

		assetManager.DumpMesh(meshID);

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

		auto mesh = ecs.GetComponent<MeshComponent>(gameObj1);

		m_Core.Renderer().GetBatchRenderer().SubmitMesh(mesh);

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

		constexpr std::wstring_view TexturePath = ENGINE_EDITOR_RESOURCES_TEXTURE_DIRECTORYW L"/basic_texture.png";
		std::filesystem::path texturePath(TexturePath);

		Asset::AssetID textureID;
		result = assetManager.LoadTexture(texturePath, textureID);
		CM_ENGINE_ASSERT(result.Succeeded());

		ConstView<Asset::Texture> textureAsset;
		assetManager.GetTexture(textureID, textureAsset);
		CM_ENGINE_ASSERT(textureAsset.NonNull());

		Resource<ITexture> texture = platform.GetGraphics().CreateTexture(
			std::span<std::byte>(textureAsset->pBuffer.get(), textureAsset->SizeBytes)
		);

		ecs.EmplaceComponent<TextureComponent>(gameObj2, textureID, std::move(texture));
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

			Renderer::Renderer& renderer = m_Core.Renderer();
			ECS::ECS& ecs = m_Core.ECS();
			Scene::SceneManager& sceneManager = m_Core.SceneManager();
			Scene::Scene& scene = sceneManager.RetrieveScene(m_EditorSceneID);

			renderer.StartFrame(Color4::Black());

			Renderer::BatchRenderer& batchRenderer = renderer.GetBatchRenderer();

			batchRenderer.BeginBatch();

			for (const auto& node : scene.Graph().Root().Nodes)
				if (node.Type == Scene::Node::NodeType::GameObject)
				{
					auto mesh = ecs.GetComponent<MeshComponent>(node.Entity);
					auto material = ecs.GetComponent<MaterialComponent>(node.Entity);
					auto texture = ecs.TryGetComponent<TextureComponent>(node.Entity);

					batchRenderer.SubmitInstance(
						node.Entity,
						mesh.ID,
						material.ID,
						texture.NonNull() ? texture->ID : Asset::AssetID()
					);
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