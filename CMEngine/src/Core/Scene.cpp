#include "Core/PCH.hpp"
#include "Core/Scene.hpp"

namespace CMEngine::Core
{
	using Vertex3 = Common::Float3;

	IScene::IScene(
		Common::LoggerWide& logger,
		ECS& ecs,
		Asset::AssetManager& assetManager,
		DX::DX11::Renderer& renderer,
		std::function<void(float)> onUpdateFunc
	) noexcept
		: m_Logger(logger),
		  m_ECS(ecs),
		  m_AssetManager(assetManager),
		  m_Renderer(renderer),
		  m_OnUpdateFunc(onUpdateFunc)
	{
	}

	void IScene::OnUpdate(float deltaTime) noexcept
	{
		m_OnUpdateFunc(deltaTime);
	}

	TestScene::TestScene(
		Common::LoggerWide& logger,
		ECS& ecs,
		Asset::AssetManager& assetManager,
		DX::DX11::Renderer& renderer
	) noexcept
		: IStockScene(
			logger,
			ecs,
			assetManager,
			renderer,
			[this](float deltaTime) {
				OnUpdate(deltaTime);
			}
		  )
	{
	}

	void TestScene::OnStart() noexcept
	{
		constexpr std::wstring_view FuncTag = L"TestScene [OnStart] | ";

		Asset::AMResultHandle resultLoad = m_AssetManager.LoadMesh("QuadMesh");

		uint16_t result = resultLoad.Result.ToInt16();
		Asset::AssetHandle handle = resultLoad.Other;

		m_Logger.LogFatalNLFormattedIf(
			!resultLoad.Result,
			FuncTag,
			L"TestScene [OnStart] | Failed to load quad mesh : `{}`.",
			result
		);

		m_CameraEntity = m_ECS.CreateEntity();

		m_Logger.LogFatalNLIf(
			!m_ECS.EmplaceComponent<RigidTransformComponent>(
				m_CameraEntity,
				Common::RigidTransform(
					Common::Float3(),
					Common::Float3(0.0f, 0.0f, -10.0f)
				)
			),
			L"TestScene [OnStart] | Failed to emplace transform on camera entity."
		);

		m_MeshEntity = m_ECS.CreateEntity();

		m_Logger.LogFatalNLIf(
			!m_ECS.EmplaceComponent<MeshComponent>(
				m_MeshEntity, 
				Common::Transform(
					Common::Float3(3.0f, 3.0f, 1.0f),
					Common::Float3(),
					Common::Float3(0.0f, 0.0f, 0.0f)
				),
				handle,
				DX::DX11::ShaderSetType::CIRCLE
			),
			L"TestScene [OnStart] | Failed to emplace mesh component on mesh entity."
		);

		Common::RigidTransform rigidTransform;
		//m_Renderer.SetCamera(CameraData(rigidTransform, 45.0f, 0.5f, 100.0f));
	}

	void TestScene::OnEnd() noexcept
	{
		m_Logger.LogFatalNLIf(
			!(m_ECS.DestroyEntity(m_CameraEntity) || m_ECS.DestroyEntity(m_MeshEntity)),
			L"TestScene [OnEnd] | Failed to destroy entities."
		);
	}

	void TestScene::OnUpdate(float deltaTime) noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Renderer.IsInitialized(),
			L"TestScene [OnUpdate] | DXContext isn't initialized."
		);

		MeshComponent* pMeshComponent = m_ECS.GetComponent<MeshComponent>(m_MeshEntity);

		m_Logger.LogFatalNLIf(
			pMeshComponent == nullptr,
			L"TestScene [OnUpdate] | Failed to retrieve mesh component."
		);

		RigidTransformComponent* pCameraTransformComponent = m_ECS.GetComponent<RigidTransformComponent>(m_CameraEntity);

		m_Logger.LogFatalNLIf(
			pCameraTransformComponent == nullptr,
			L"TestScene [OnUpdate] | Failed to retrieve camera transform component."
		);

		// Here so Intelisense doesn't yell at me for de-referencing a null pointer, even though fatal logs terminate the program.
		if (pMeshComponent == nullptr || pCameraTransformComponent == nullptr)
			return;

		// Update model matrix if transform changed.
		if (!pMeshComponent->Transform.IsNearEqual(m_PreviousMeshTransform))
		{
			/*m_Renderer.SetModelMatrix(
				DirectX::XMMatrixScaling(
					pMeshComponent->Transform.Scaling.x,
					pMeshComponent->Transform.Scaling.y,
					pMeshComponent->Transform.Scaling.z
				) *
				DirectX::XMMatrixRotationRollPitchYaw(
					pMeshComponent->Transform.Rotation.x,
					pMeshComponent->Transform.Rotation.y,
					pMeshComponent->Transform.Rotation.z
				) *
				DirectX::XMMatrixTranslation(
					pMeshComponent->Transform.Translation.x,
					pMeshComponent->Transform.Translation.y,
					pMeshComponent->Transform.Translation.z
				)
			);*/

			m_PreviousMeshTransform = pMeshComponent->Transform;
		}

		if (m_Renderer.CurrentShaderSet() != pMeshComponent->SetType)
			m_Renderer.SetShaderSet(pMeshComponent->SetType);

		// Update camera if transform changed.
		if (!pCameraTransformComponent->IsNearEqual(m_PreviousCameraTransform))
		{
			//m_Renderer.SetCameraTransform(pCameraTransformComponent->RigidTransform);
			m_PreviousCameraTransform = pCameraTransformComponent->RigidTransform;
		}

		Asset::AMResultPair<const Asset::Mesh*> meshResult = m_AssetManager.GetMesh(pMeshComponent->Handle);
		const Asset::Mesh* pMesh = meshResult.Other;

		m_Logger.LogFatalNLIf(pMesh == nullptr, L"TestScene [OnUpdate] | Failed to retrieve mesh asset.");

		std::array<float, 1> instanceRadii = { 0.5f };

		//renderContextRef.DrawIndexed<Vertex3>(pMeshComponent->Vertices, pMeshComponent->Indices);
		DX::DX11::DrawDescriptor descriptor;

		descriptor.TotalVertices = pMesh->Data.Descriptor.VertexCount;
		descriptor.TotalIndices = pMesh->Data.Descriptor.IndexCount;
		descriptor.TotalInstances = static_cast<UINT>(instanceRadii.size()),
		descriptor.VertexByteStride = pMesh->Data.Descriptor.VertexByteStride;
		descriptor.IndexByteStride = pMesh->Data.Descriptor.IndexByteStride;

		/*m_Renderer.DrawIndexedInstanced<const std::byte, const std::byte, float>(
			pMesh->Data.VertexData,
			pMesh->Data.IndexData,
			instanceRadii,
			descriptor
		);*/

		ShowMeshWindow(pMeshComponent->Transform);
		ShowCameraWindow(pCameraTransformComponent->RigidTransform);

		/*RECT clientArea = m_RendererRef.Window().ClientArea();

		D2D1_RECT_F	rectF = {};
		rectF.left = clientArea.left;
		rectF.top = clientArea.top;
		rectF.right = clientArea.right;
		rectF.bottom = clientArea.bottom;

		m_RendererRef.RenderContext().Writer().WriteText(L"HELLO!!!", rectF.left, rectF.top, rectF.right, rectF.bottom);*/
	}

	void TestScene::ShowMeshWindow(Common::Transform& meshTransform) noexcept
	{
		// Full width, fixed height (scrollable if content overflows)
		ImVec2 childSize(0.0f, 150.0f);

		m_Renderer.ImGuiBegin("Mesh Transform");

		if (m_Renderer.ImGuiCollapsingHeader("Scaling"))
		{
			// Begin a child region inside the collapsing header
			m_Renderer.ImGuiBeginChild("ChildScaling", childSize, ImGuiChildFlags_Border);

			m_Renderer.ImGuiSlider("X", &meshTransform.Scaling.x, 0.01f, 20.0f);
			m_Renderer.ImGuiSlider("Y", &meshTransform.Scaling.y, 0.01f, 20.0f);
			m_Renderer.ImGuiSlider("Z", &meshTransform.Scaling.z, 0.01f, 20.0f);

			m_Renderer.ImGuiEndChild();
		}
		if (m_Renderer.ImGuiCollapsingHeader("Rotation"))
		{
			// Begin a child region inside the collapsing header
			m_Renderer.ImGuiBeginChild("ChildRotation", childSize, ImGuiChildFlags_Border);

			m_Renderer.ImGuiSliderAngle("X", &meshTransform.Rotation.x, -360.0f, 360.0f);
			m_Renderer.ImGuiSliderAngle("Y", &meshTransform.Rotation.y, -360.0f, 360.0f);
			m_Renderer.ImGuiSliderAngle("Z", &meshTransform.Rotation.z, -360.0f, 360.0f);

			m_Renderer.ImGuiEndChild();
		}
		if (m_Renderer.ImGuiCollapsingHeader("Translation"))
		{
			// Begin a child region inside the collapsing header
			m_Renderer.ImGuiBeginChild("ChildTranslation", childSize, ImGuiChildFlags_Border);

			m_Renderer.ImGuiSlider("X", &meshTransform.Translation.x, -20, 20);
			m_Renderer.ImGuiSlider("Y", &meshTransform.Translation.y, -20, 20);
			m_Renderer.ImGuiSlider("Z", &meshTransform.Translation.z, -20, 20);

			m_Renderer.ImGuiEndChild();
		}

		m_Renderer.ImGuiEnd();
	}

	void TestScene::ShowCameraWindow(Common::RigidTransform& cameraTransform) noexcept
	{
		// Full width, fixed height (scrollable if content overflows)
		ImVec2 childSize(0.0f, 150.0f);

		m_Renderer.ImGuiBegin("Camera Transform");

		if (m_Renderer.ImGuiCollapsingHeader("Rotation"))
		{
			// Begin a child region inside the collapsing header
			m_Renderer.ImGuiBeginChild("ChildRotation", childSize, ImGuiChildFlags_Border);

			m_Renderer.ImGuiSliderAngle("X", &cameraTransform.Rotation.x, -360.0f, 360.0f);
			m_Renderer.ImGuiSliderAngle("Y", &cameraTransform.Rotation.y, -360.0f, 360.0f);
			m_Renderer.ImGuiSliderAngle("Z", &cameraTransform.Rotation.z, -360.0f, 360.0f);

			m_Renderer.ImGuiEndChild();
		}
		if (m_Renderer.ImGuiCollapsingHeader("Translation"))
		{
			// Begin a child region inside the collapsing header
			m_Renderer.ImGuiBeginChild("ChildTranslation", childSize, ImGuiChildFlags_Border);

			m_Renderer.ImGuiSlider("X", &cameraTransform.Translation.x, -20, 20);
			m_Renderer.ImGuiSlider("Y", &cameraTransform.Translation.y, -20, 20);
			m_Renderer.ImGuiSlider("Z", &cameraTransform.Translation.z, -20, 20);

			m_Renderer.ImGuiEndChild();
		}

		m_Renderer.ImGuiEnd();
	}
}