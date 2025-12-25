#include "Editor.hpp"
#include "Component.hpp"
#include "Math.hpp"

#include <iostream>
#include <chrono>
#include <thread>
#include <format>

namespace std
{
	template <>
	struct formatter<CMEngine::Float3>
	{
		// Parse: In this simple case, we don't support custom format specifiers,
		// so we just consume the closing '}'
		constexpr auto parse(std::format_parse_context& ctx)
		{
			return ctx.begin();
		}

		auto format(const CMEngine::Float3& f, auto& ctx) const
		{
			return std::format_to(ctx.out(), "({}, {}, {})", f.x, f.y, f.z);
		}
	};

	template <>
	struct formatter<CMEngine::Float3, wchar_t>
	{
		// Parse: In this simple case, we don't support custom format specifiers,
		// so we just consume the closing '}'
		constexpr auto parse(auto& ctx)
		{
			return ctx.begin();
		}

		auto format(const CMEngine::Float3& f, auto& ctx) const
		{
			return std::format_to(ctx.out(), L"({}, {}, {})", f.x, f.y, f.z);
		}
	};
}

namespace CMEngine::Editor
{
	Editor::Editor() noexcept
	{
		ECS::ECS& ecs = m_Core.ECS();

		ECS::Entity cameraEntity = ecs.CreateEntity();
		ECS::Entity gameObj1 = ecs.CreateEntity();
		ECS::Entity gameObj2 = ecs.CreateEntity();
			
		Asset::AssetManager& assetManager = m_Core.AssetManager();

		constexpr std::string_view MeshName = ENGINE_EDITOR_RESOURCES_MODEL_DIRECTORY "/test_cube.glb";

		/* TODO: Fix weird DeadlyImportError exception... */
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
		ecs.EmplaceComponent<LocomotionComponent>(cameraEntity);
		ecs.EmplaceComponent<TransformComponent>(gameObj1);
		ecs.EmplaceComponent<TransformComponent>(gameObj2);

		/* TODO: Clean this manual event parsing up... (input mapping system?) */
		m_KeyPressedID = m_Core.EventSystem().Subscribe(
			Event::EventType::Input_Key_Pressed,
			[cameraEntity, &ecs](const Event::IEvent& event)
			{
				CM_ENGINE_ASSERT(TryCast<const Event::KeyPressed*>(&event) != nullptr);
				
				auto& keyPressed = *static_cast<const Event::KeyPressed*>(&event);
				auto locomotion = ecs.TryGetComponent<LocomotionComponent>(cameraEntity);

				if (locomotion.Null())
					return;

				switch (keyPressed.Keycode)
				{
				case KeycodeType::Shift:
					locomotion->SetFlag(LocomotionState::Running);
					locomotion->ClearFlag(LocomotionState::Walking);
					break;
				case KeycodeType::Ctrl:
					locomotion->InputDirection.y += -1.0f;
					break;
				case KeycodeType::Space:
					locomotion->InputDirection.y += 1.0f;
					break;
				case KeycodeType::W:
					locomotion->InputDirection.z += 1.0f;
					break;
				case KeycodeType::A: 
					locomotion->InputDirection.x += -1.0f;
					break;
				case KeycodeType::S: 
					locomotion->InputDirection.z += -1.0f;
					break;
				case KeycodeType::D: 
					locomotion->InputDirection.x += 1.0f;
					break;
				case KeycodeType::B:
					CM_ENGINE_BREAK_DEBUGGER();
					break;
				default:
					return;
				}
			});

		m_KeyReleasedID = m_Core.EventSystem().Subscribe(
			Event::EventType::Input_Key_Released,
			[cameraEntity, &ecs](const Event::IEvent& event)
			{
				CM_ENGINE_ASSERT(TryCast<const Event::KeyReleased*>(&event) != nullptr);

				auto keyPressed = *static_cast<const Event::KeyReleased*>(&event);
				auto locomotion = ecs.TryGetComponent<LocomotionComponent>(cameraEntity);

				switch (keyPressed.Keycode)
				{
				case KeycodeType::Shift:
					locomotion->ClearFlag(LocomotionState::Running);
					locomotion->SetFlag(LocomotionState::Walking);
					break;
				case KeycodeType::Ctrl:
					locomotion->InputDirection.y += 1.0f;
					break;
				case KeycodeType::Space:
					locomotion->InputDirection.y += -1.0f;
					break;
				case KeycodeType::W:
					locomotion->InputDirection.z += -1.0f;
					break;
				case KeycodeType::A:
					locomotion->InputDirection.x += 1.0f;
					break;
				case KeycodeType::S:
					locomotion->InputDirection.z += 1.0f;
					break;
				case KeycodeType::D:
					locomotion->InputDirection.x += -1.0f;
					break;
				default:
					return;
				}
			});

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
		/* Unsubscribe any listeners so no callbacks are triggered after destruction. */
		auto& eventSystem = m_Core.EventSystem();
		eventSystem.Unsubscribe(m_KeyPressedID);
		eventSystem.Unsubscribe(m_KeyReleasedID);
	}


	void Editor::Run() noexcept
	{
		auto lastTime = std::chrono::high_resolution_clock::now();
		
		constexpr std::string_view DirectionFmt =	  "Input Direction: {}";
		constexpr std::string_view NormDirectionFmt = "Normalized Direction: {}";
		constexpr std::string_view WalkSpeedFmt =	  "Walk Speed: {}";
		constexpr std::string_view VelocityFmt =	  "Velocity: {}";
		constexpr std::string_view DeltaVelocityFmt = "Delta Velocity: {}";
		constexpr std::string_view PosFmt =			  "Pos: {}";

		while (m_Core.Platform().IsRunning())
		{
			auto startTime = std::chrono::high_resolution_clock::now();

			float deltaSeconds = std::chrono::duration<float>((startTime - lastTime)).count();

			lastTime = startTime;

			ECS::ECS& ecs = m_Core.ECS();
			Scene::SceneManager& sceneManager = m_Core.SceneManager();
			Renderer::Renderer& renderer = m_Core.Renderer();

			/* Process window messages, etc... */
			m_Core.Update();

			ECS::Entity cameraEntity = sceneManager.GetCameraSystem().GetMainCameraEntity();

			auto& locomotion = ecs.GetComponent<LocomotionComponent>(cameraEntity);
			auto& camera = ecs.GetComponent<CameraComponent>(cameraEntity);

			Float3& currentPos = camera.Data.Origin;
			Float3 previousPos = currentPos;

			Float3 normDirection = Math::Normalize(locomotion.InputDirection);
			Float3 velocity = normDirection;

			velocity.x *= locomotion.WalkSpeed;
			velocity.z *= locomotion.WalkSpeed;
			velocity.y *= locomotion.JumpForce;

			Float3 deltaVelocity = velocity * deltaSeconds;
			currentPos += deltaVelocity;

			/* Technically, the input direction from last frame could be cached 
			 *   instead, saving the calulations unless necessary... */
			if (!currentPos.IsNearEqual(previousPos))
			{
				camera.CreateViewMatrix();
				renderer.SetCamera(camera);
			}

			renderer.StartFrame(Color4::Black());

			if (renderer.ImGuiWindow("Locomotion Debugging"))
			{
				std::string directionStr = std::vformat(DirectionFmt, std::make_format_args(locomotion.InputDirection));
				std::string normDirectionStr = std::vformat(NormDirectionFmt, std::make_format_args(normDirection));
				std::string walkSpeedStr = std::vformat(WalkSpeedFmt, std::make_format_args(locomotion.WalkSpeed));
				std::string velocityStr = std::vformat(VelocityFmt, std::make_format_args(velocity));
				std::string deltaVelocityStr = std::vformat(DeltaVelocityFmt, std::make_format_args(deltaVelocity));
				std::string posStr = std::vformat(PosFmt, std::make_format_args(currentPos));

				renderer.ImGuiText(directionStr);
				renderer.ImGuiText(normDirectionStr);
				renderer.ImGuiText(walkSpeedStr);
				renderer.ImGuiText(velocityStr);
				renderer.ImGuiText(deltaVelocityStr);
				renderer.ImGuiText(posStr);
			}

			renderer.ImGuiEndWindow();

			Renderer::BatchRenderer& batchRenderer = renderer.GetBatchRenderer();

			batchRenderer.BeginBatch();

			Scene::Scene& scene = sceneManager.RetrieveScene(m_EditorSceneID);
			for (const auto& node : scene.Graph().Root().Nodes)
				if (node.Type == Scene::Node::NodeType::GameObject)
				{
					MeshComponent mesh = ecs.GetComponent<MeshComponent>(node.Entity);
					MaterialComponent material = ecs.GetComponent<MaterialComponent>(node.Entity);
					View<TextureComponent> texture = ecs.TryGetComponent<TextureComponent>(node.Entity);

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