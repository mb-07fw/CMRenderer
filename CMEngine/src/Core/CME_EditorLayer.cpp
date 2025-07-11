#include "Core/CME_PCH.hpp"
#include "Core/CME_EditorLayer.hpp"
#include "Core/CME_Component.hpp"

namespace CMEngine
{
	CMEditorLayer::CMEditorLayer(
		CMCommon::CMLoggerWide& logger,
		DirectXAPI::DX11::DXRenderer& renderer,
		CMCommon::CMECS& ecs,
		Asset::CMAssetManager& assetManager,
		CMSceneManager& sceneManager,
		CMCommon::CMMetaArena& engineHeap
	) noexcept
		: ICMLayer(logger, renderer),
		  m_ECS(ecs),
		  m_AssetManager(assetManager),
		  m_SceneManager(sceneManager),
		  m_EngineHeap(engineHeap)
	{
	}

	void CMEditorLayer::OnAttach() noexcept
	{
		constexpr std::wstring_view FuncTag = L"CMEditorLayer [OnAttach] | ";
		constexpr std::string_view MeshName = "QuadMesh";
		constexpr std::wstring_view MeshNameW = L"QuadMesh";

		Asset::CMMeshData meshData;
		meshData.Descriptor.MeshName = "TestQuadMesh";
		meshData.Descriptor.VertexCount = 4;
		meshData.Descriptor.IndexCount = 6;
		meshData.Descriptor.VertexByteStride = ToByteSize(CMTypeHint::FLOAT32x2);
		meshData.Descriptor.IndexByteStride = 2;
		meshData.Descriptor.VertexByteStrideHint = CMTypeHint::FLOAT32x2;
		meshData.Descriptor.IndexByteStrideHint = CMTypeHint::UINT16;
		meshData.Descriptor.Attributes.emplace_back("UV_Pos2D", CMTypeHint::FLOAT32x2, 0);

		constexpr size_t QuadVerticesByteSize = sizeof(Asset::CMMeshConstants::S_QUAD_FRONT_VERTICES);
		constexpr size_t QuadIndicesByteSize = sizeof(Asset::CMMeshConstants::S_QUAD_FRONT_INDICES);

		const std::byte* pQuadVertices = reinterpret_cast<const std::byte*>(
			Asset::CMMeshConstants::S_QUAD_FRONT_VERTICES.data()
		);

		const std::byte* pQuadIndices = reinterpret_cast<const std::byte*>(
			Asset::CMMeshConstants::S_QUAD_FRONT_INDICES.data()
		);

		meshData.VertexData.assign(
			pQuadVertices,
			pQuadVertices + QuadVerticesByteSize
		);

		meshData.IndexData.assign(
			pQuadIndices,
			pQuadIndices + QuadIndicesByteSize
		);

		Asset::CMAMResultHandle meshHandle = m_AssetManager.SaveMeshAsNew(meshData, MeshName);

		m_CameraEntity = m_ECS.CreateEntity();
		m_GUIEntity = m_ECS.CreateEntity();

		m_Logger.LogFatalNLFormattedIf(
			meshHandle.Result.Failed(),
			FuncTag,
			L"Failed to retrieve mesh : `{}`.",
			MeshNameW
		);

		bool emplaced = m_ECS.EmplaceComponent<CMMeshComponent>(
			m_GUIEntity,
			CMCommon::CMTransform(),
			meshHandle.Other,
			DirectXAPI::DX11::DXShaderSetType::QUAD_OUTLINED
		);

		m_Logger.LogFatalNLTaggedIf(
			!emplaced,
			FuncTag,
			L"Failed to emplace mesh component on GUI entity."
		);

		m_PreviousCameraTransform = CMCommon::CMRigidTransform(
			CMCommon::CMFloat3(), CMCommon::CMFloat3(0.0f, 0.0f, -10.0f)
		);

		emplaced = m_ECS.EmplaceComponent<CMCameraComponent>(
			m_CameraEntity,
			m_PreviousCameraTransform
		);

		m_Logger.LogFatalNLTaggedIf(
			!emplaced,
			FuncTag,
			L"Failed to emplace camera component on camera entity."
		);

		CMCommon::CMRect renderArea = m_Renderer.CurrentRenderArea();

		CMCameraData cameraData(
			m_PreviousCameraTransform,
			45.0f,
			0.5f,
			100.0f
		);

		m_Renderer.SetCamera(cameraData);
		m_Renderer.SetShaderSet(DirectXAPI::DX11::DXShaderSetType::QUAD_OUTLINED);
	}

	void CMEditorLayer::OnDetach() noexcept
	{
		CMMeshComponent* pGuiMeshComponent = m_ECS.GetComponent<CMMeshComponent>(m_GUIEntity);

		if (pGuiMeshComponent != nullptr)
			m_AssetManager.Unregister(pGuiMeshComponent->Handle);

		m_ECS.DestroyEntity(m_CameraEntity);
		m_ECS.DestroyEntity(m_GUIEntity);
	}

	void CMEditorLayer::OnUpdate(float deltaTime) noexcept
	{
		constexpr std::wstring_view FuncTag = L"CMEditorLayer [OnUpdate] | ";

		CMCameraComponent* pCameraComponent = m_ECS.GetComponent<CMCameraComponent>(m_CameraEntity);
		CMMeshComponent* pMeshComponent = m_ECS.GetComponent<CMMeshComponent>(m_GUIEntity);

		m_Logger.LogFatalNLTaggedIf(
			pMeshComponent == nullptr,
			FuncTag,
			L"Failed to retrieve GUI entity's mesh component."
		);

		/* Here so visual studio doesn't yell at me for de-referencing a
		 * null pointer even though fatal logs terminate the program.
		 */
		if (pMeshComponent == nullptr)
			return;

		Asset::CMAMResultPair<const Asset::CMMesh*> meshResult = m_AssetManager.GetMesh(pMeshComponent->Handle);
		const Asset::CMMesh* pMesh = meshResult.Other;

		m_Logger.LogFatalNLTaggedIf(
			meshResult.Result.Failed(),
			FuncTag,
			L"Failed to retrieve GUI entity's mesh."
		);

		/* Here so visual studio doesn't yell at me for de-referencing a
		 * null pointer even though fatal logs terminate the program.
		 */
		if (pMesh == nullptr)
			return;

		CMCommon::CMTransform& currentMeshTransform = pMeshComponent->Transform;
		CMCommon::CMRigidTransform& currentCameraTransform = pCameraComponent->RigidTransform;

		if (!currentMeshTransform.IsNearEqual(m_PreviousMeshTransform))
		{
			DirectX::XMMATRIX updatedModelMatrix = DirectX::XMMatrixScaling(
				currentMeshTransform.Scaling.x,
				currentMeshTransform.Scaling.y,
				currentMeshTransform.Scaling.z
			) * DirectX::XMMatrixRotationRollPitchYaw(
				currentMeshTransform.Rotation.x,
				currentMeshTransform.Rotation.y,
				currentMeshTransform.Rotation.z
			) * DirectX::XMMatrixTranslation(
				currentMeshTransform.Translation.x,
				currentMeshTransform.Translation.y,
				currentMeshTransform.Translation.z
			);

			m_Renderer.SetModelMatrix(updatedModelMatrix);
			m_PreviousMeshTransform = currentMeshTransform;
		}

		if (!currentCameraTransform.IsNearEqual(m_PreviousCameraTransform))
			m_Renderer.SetCameraTransform(currentCameraTransform);

		DirectXAPI::DX11::DXDrawDescriptor drawDescriptor;
		const Asset::CMMeshDescriptor& meshDescriptor = pMesh->Data.Descriptor;

		drawDescriptor.TotalVertices = meshDescriptor.VertexCount;
		drawDescriptor.VertexByteStride = meshDescriptor.VertexByteStride;
		drawDescriptor.TotalIndices = meshDescriptor.IndexCount;
		drawDescriptor.IndexByteStride = meshDescriptor.IndexByteStride;

		m_Renderer.DrawIndexed<std::byte, std::byte>(
			pMesh->Data.VertexData,
			pMesh->Data.IndexData,
			drawDescriptor
		);
		
		ShowCameraWindow(currentCameraTransform);
		ShowMeshWindow(currentMeshTransform);
	}

	void CMEditorLayer::ShowMeshWindow(CMCommon::CMTransform& meshTransform) noexcept
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

	void CMEditorLayer::ShowCameraWindow(CMCommon::CMRigidTransform& cameraTransform) noexcept
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