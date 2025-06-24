#include "Core/CME_PCH.hpp"
#include "Core/CME_Scene.hpp"

namespace CMEngine
{
	using Vertex3 = CMCommon::CMFloat3;

	ICMScene::ICMScene(
		CMCommon::CMLoggerWide& logger,
		CMCommon::CMECS& ecs,
		Asset::CMAssetManager& assetManager,
		DirectXAPI::DX11::DXContext& context,
		std::function<void(float)> onUpdateFunc
	) noexcept
		: m_Logger(logger),
		  m_ECS(ecs),
		  m_AssetManager(assetManager),
		  m_Context(context),
		  m_OnUpdateFunc(onUpdateFunc)
	{
	}

	void ICMScene::OnUpdate(float deltaTime) noexcept
	{
		m_OnUpdateFunc(deltaTime);
	}

	CMTestScene::CMTestScene(
		CMCommon::CMLoggerWide& logger,
		CMCommon::CMECS& ecs,
		Asset::CMAssetManager& assetManager,
		DirectXAPI::DX11::DXContext& context
	) noexcept
		: ICMStockScene(
			logger,
			ecs,
			assetManager,
			context,
			[this](float deltaTime) {
				OnUpdate(deltaTime);
			}
		  )
	{
	}

	void CMTestScene::OnStart() noexcept
	{
		constexpr std::wstring_view FuncTag = L"CMTestScene [OnStart] | ";

		Asset::CMAMResultHandle resultLoad = m_AssetManager.LoadMesh("QuadMesh");

		uint16_t result = resultLoad.Result.ToInt16();
		Asset::CMAssetHandle handle = resultLoad.Other;

		m_Logger.LogFatalNLFormattedIf(
			!resultLoad.Result,
			FuncTag,
			L"CMTestScene [OnStart] | Failed to load quad mesh : `{}`.",
			result
		);

		m_CameraEntity = m_ECS.CreateEntity();

		m_Logger.LogFatalNLIf(
			!m_ECS.EmplaceComponent<CMRigidTransformComponent>(
				m_CameraEntity,
				CMCommon::CMRigidTransform(
					CMCommon::CMFloat3(),
					CMCommon::CMFloat3(0.0f, 0.0f, -10.0f)
				)
			),
			L"CMTestScene [OnStart] | Failed to emplace transform on camera entity."
		);

		m_MeshEntity = m_ECS.CreateEntity();

		m_Logger.LogFatalNLIf(
			!m_ECS.EmplaceComponent<CMMeshComponent>(
				m_MeshEntity, 
				CMCommon::CMTransform(
					CMCommon::CMFloat3(3.0f, 3.0f, 1.0f),
					CMCommon::CMFloat3(),
					CMCommon::CMFloat3(0.0f, 0.0f, 0.0f)
				),
				handle,
				DirectXAPI::DX11::DXShaderSetType::CIRCLE
			),
			L"CMTestScene [OnStart] | Failed to emplace mesh component on mesh entity."
		);

		CMCommon::CMRigidTransform rigidTransform;
		m_Context.SetCamera(CMCameraData(rigidTransform, 45.0f, 0.5f, 100.0f));
	}

	void CMTestScene::OnEnd() noexcept
	{
		m_Logger.LogFatalNLIf(
			!(m_ECS.DestroyEntity(m_CameraEntity) || m_ECS.DestroyEntity(m_MeshEntity)),
			L"CMTestScene [OnEnd] | Failed to destroy entities."
		);
	}

	void CMTestScene::OnUpdate(float deltaTime) noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Context.IsInitialized(),
			L"CMTestScene [OnUpdate] | DXContext isn't initialized."
		);

		CMMeshComponent* pMeshComponent = m_ECS.GetComponent<CMMeshComponent>(m_MeshEntity);

		m_Logger.LogFatalNLIf(
			pMeshComponent == nullptr,
			L"CMTestScene [OnUpdate] | Failed to retrieve mesh component."
		);

		CMRigidTransformComponent* pCameraTransformComponent = m_ECS.GetComponent<CMRigidTransformComponent>(m_CameraEntity);

		m_Logger.LogFatalNLIf(
			pCameraTransformComponent == nullptr,
			L"CMTestScene [OnUpdate] | Failed to retrieve camera transform component."
		);

		// Here so Intelisense doesn't yell at me for de-referencing a null pointer, even though fatal logs terminate the program.
		if (pMeshComponent == nullptr || pCameraTransformComponent == nullptr)
			return;

		// Update model matrix if transform changed.
		if (!pMeshComponent->Transform.IsNearEqual(m_PreviousMeshTransform))
		{
			m_Context.SetModelMatrix(
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
			);

			m_PreviousMeshTransform = pMeshComponent->Transform;
		}

		if (m_Context.CurrentShaderSet() != pMeshComponent->SetType)
			m_Context.SetShaderSet(pMeshComponent->SetType);

		// Update camera if transform changed.
		if (!pCameraTransformComponent->IsNearEqual(m_PreviousCameraTransform))
		{
			m_Context.SetCameraTransform(pCameraTransformComponent->RigidTransform);
			m_PreviousCameraTransform = pCameraTransformComponent->RigidTransform;
		}

		Asset::CMAMResultPair<const Asset::CMMesh*> meshResult = m_AssetManager.GetMesh(pMeshComponent->Handle);
		const Asset::CMMesh* pMesh = meshResult.Other;

		m_Logger.LogFatalNLIf(pMesh == nullptr, L"CMTestScene [OnUpdate] | Failed to retrieve mesh asset.");

		std::array<float, 1> instanceRadii = { 0.5f };

		//renderContextRef.DrawIndexed<Vertex3>(pMeshComponent->Vertices, pMeshComponent->Indices);
		DirectXAPI::DX11::DXDrawDescriptor descriptor;

		descriptor.TotalVertices = pMesh->Data.Descriptor.VertexCount;
		descriptor.TotalIndices = pMesh->Data.Descriptor.IndexCount;
		descriptor.TotalInstances = static_cast<UINT>(instanceRadii.size()),
		descriptor.VertexByteStride = pMesh->Data.Descriptor.VertexByteStride;
		descriptor.IndexByteStride = pMesh->Data.Descriptor.IndexByteStride;

		m_Context.DrawIndexedInstanced<const std::byte, const std::byte, float>(
			pMesh->Data.VertexData,
			pMesh->Data.IndexData,
			instanceRadii,
			descriptor
		);

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

	void CMTestScene::ShowMeshWindow(CMCommon::CMTransform& meshTransform) noexcept
	{
		// Full width, fixed height (scrollable if content overflows)
		ImVec2 childSize(0.0f, 150.0f);

		m_Context.ImGuiBegin("Mesh Transform");

		if (m_Context.ImGuiCollapsingHeader("Scaling"))
		{
			// Begin a child region inside the collapsing header
			m_Context.ImGuiBeginChild("ChildScaling", childSize, ImGuiChildFlags_Border);

			m_Context.ImGuiSlider("X", &meshTransform.Scaling.x, 0.01f, 20.0f);
			m_Context.ImGuiSlider("Y", &meshTransform.Scaling.y, 0.01f, 20.0f);
			m_Context.ImGuiSlider("Z", &meshTransform.Scaling.z, 0.01f, 20.0f);

			m_Context.ImGuiEndChild();
		}
		if (m_Context.ImGuiCollapsingHeader("Rotation"))
		{
			// Begin a child region inside the collapsing header
			m_Context.ImGuiBeginChild("ChildRotation", childSize, ImGuiChildFlags_Border);

			m_Context.ImGuiSliderAngle("X", &meshTransform.Rotation.x, -360.0f, 360.0f);
			m_Context.ImGuiSliderAngle("Y", &meshTransform.Rotation.y, -360.0f, 360.0f);
			m_Context.ImGuiSliderAngle("Z", &meshTransform.Rotation.z, -360.0f, 360.0f);

			m_Context.ImGuiEndChild();
		}
		if (m_Context.ImGuiCollapsingHeader("Translation"))
		{
			// Begin a child region inside the collapsing header
			m_Context.ImGuiBeginChild("ChildTranslation", childSize, ImGuiChildFlags_Border);

			m_Context.ImGuiSlider("X", &meshTransform.Translation.x, -20, 20);
			m_Context.ImGuiSlider("Y", &meshTransform.Translation.y, -20, 20);
			m_Context.ImGuiSlider("Z", &meshTransform.Translation.z, -20, 20);

			m_Context.ImGuiEndChild();
		}

		m_Context.ImGuiEnd();
	}

	void CMTestScene::ShowCameraWindow(CMCommon::CMRigidTransform& cameraTransform) noexcept
	{
		// Full width, fixed height (scrollable if content overflows)
		ImVec2 childSize(0.0f, 150.0f);

		m_Context.ImGuiBegin("Camera Transform");

		if (m_Context.ImGuiCollapsingHeader("Rotation"))
		{
			// Begin a child region inside the collapsing header
			m_Context.ImGuiBeginChild("ChildRotation", childSize, ImGuiChildFlags_Border);

			m_Context.ImGuiSliderAngle("X", &cameraTransform.Rotation.x, -360.0f, 360.0f);
			m_Context.ImGuiSliderAngle("Y", &cameraTransform.Rotation.y, -360.0f, 360.0f);
			m_Context.ImGuiSliderAngle("Z", &cameraTransform.Rotation.z, -360.0f, 360.0f);

			m_Context.ImGuiEndChild();
		}
		if (m_Context.ImGuiCollapsingHeader("Translation"))
		{
			// Begin a child region inside the collapsing header
			m_Context.ImGuiBeginChild("ChildTranslation", childSize, ImGuiChildFlags_Border);

			m_Context.ImGuiSlider("X", &cameraTransform.Translation.x, -20, 20);
			m_Context.ImGuiSlider("Y", &cameraTransform.Translation.y, -20, 20);
			m_Context.ImGuiSlider("Z", &cameraTransform.Translation.z, -20, 20);

			m_Context.ImGuiEndChild();
		}

		m_Context.ImGuiEnd();
	}
}