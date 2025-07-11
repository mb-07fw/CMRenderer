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

		m_GUIEntity = m_ECS.CreateEntity();

		//Asset::CMAMResultHandle meshHandle = m_AssetManager.LoadMesh(MeshName);

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

		CMCommon::CMRect renderArea = m_Renderer.CurrentRenderArea();

		CMCameraData cameraData(
			CMCommon::CMRigidTransform(),
			CMOrthographicParams(renderArea),
			0.5f,
			100.0f
		);

		m_Renderer.SetCamera(cameraData);
		m_Renderer.SetShaderSet(DirectXAPI::DX11::DXShaderSetType::QUAD_OUTLINED);
		m_Renderer.SetModelMatrix(DirectX::XMMatrixScaling(2.0f, 2.0f, 1.0f));
	}

	void CMEditorLayer::OnDetach() noexcept
	{
		CMMeshComponent* pGuiMeshComponent = m_ECS.GetComponent<CMMeshComponent>(m_GUIEntity);

		if (pGuiMeshComponent != nullptr)
			m_AssetManager.Unregister(pGuiMeshComponent->Handle);

		m_ECS.DestroyEntity(m_GUIEntity);
	}

	void CMEditorLayer::OnUpdate(float deltaTime) noexcept
	{
		constexpr std::wstring_view FuncTag = L"CMEditorLayer [OnUpdate] | ";

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

		DirectXAPI::DX11::DXDrawDescriptor drawDescriptor;

		drawDescriptor.TotalVertices = pMesh->Data.Descriptor.VertexCount;
		drawDescriptor.VertexByteStride = pMesh->Data.Descriptor.VertexByteStride;
		drawDescriptor.TotalIndices = pMesh->Data.Descriptor.IndexCount;
		drawDescriptor.IndexByteStride = pMesh->Data.Descriptor.IndexByteStride;

		m_Renderer.DrawIndexed<std::byte, std::byte>(
			pMesh->Data.VertexData,
			pMesh->Data.IndexData,
			drawDescriptor
		);
	}
}