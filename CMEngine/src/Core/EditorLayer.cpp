#include "Core/PCH.hpp"
#include "Core/EditorLayer.hpp"
#include "Core/Component.hpp"
#include "Common/Utility.hpp"

namespace CMEngine::Core
{
	EditorLayer::EditorLayer(
		Common::LoggerWide& logger,
		DX::DX11::Renderer& renderer,
		Win::Window& window,
		ECS& ecs,
		Asset::AssetManager& assetManager,
		SceneManager& sceneManager,
		Common::MetaArena& engineHeap
	) noexcept
		: ILayer(logger, renderer, window),
		  m_ECS(ecs),
		  m_AssetManager(assetManager),
		  m_SceneManager(sceneManager),
		  m_EngineHeap(engineHeap)
	{
	}

	void EditorLayer::OnAttach() noexcept
	{
		constexpr std::wstring_view FuncTag = L"EditorLayer [OnAttach] | ";
		constexpr std::string_view MeshName = "QuadMesh";
		constexpr std::wstring_view MeshNameW = L"QuadMesh";

		Asset::MeshData meshData;
		meshData.Descriptor.MeshName = "TestQuadMesh";
		meshData.Descriptor.VertexCount = static_cast<uint32_t>(Asset::MeshConstants::S_QUAD_FRONT_VERTICES.size());
		meshData.Descriptor.IndexCount = static_cast<uint32_t>(Asset::MeshConstants::S_QUAD_FRONT_INDICES.size());
		meshData.Descriptor.VertexByteStride = sizeof(Common::Float3);
		meshData.Descriptor.IndexByteStride = 2;
		meshData.Descriptor.VertexByteStrideHint = TypeHint::NONE;
		meshData.Descriptor.IndexByteStrideHint = TypeHint::NONE;
		meshData.Descriptor.Attributes.emplace_back("Screen_Pos2D", TypeHint::NONE, 0);

		constexpr size_t QuadVerticesByteSize = sizeof(Asset::MeshConstants::S_QUAD_FRONT_DEPTH_VERTICES);
		constexpr size_t QuadIndicesByteSize = sizeof(Asset::MeshConstants::S_QUAD_FRONT_INDICES);

		const std::byte* pQuadVertices = reinterpret_cast<const std::byte*>(
			Asset::MeshConstants::S_QUAD_FRONT_DEPTH_VERTICES.data()
		);

		const std::byte* pQuadIndices = reinterpret_cast<const std::byte*>(
			Asset::MeshConstants::S_QUAD_FRONT_INDICES.data()
		);

		meshData.VertexData.assign(
			pQuadVertices,
			pQuadVertices + QuadVerticesByteSize
		);

		meshData.IndexData.assign(
			pQuadIndices,
			pQuadIndices + QuadIndicesByteSize
		);

		Asset::AMResultHandle meshHandle = m_AssetManager.SaveMeshAsNew(meshData, MeshName);

		m_CameraEntity = m_ECS.CreateEntity();
		m_GUIEntity = m_ECS.CreateEntity();

		m_Logger.LogFatalNLFormattedIf(
			meshHandle.Result.Failed(),
			FuncTag,
			L"Failed to retrieve mesh : `{}`.",
			MeshNameW
		);

		Common::Float2 renderResolution = m_Renderer.CurrentResolution();

		bool emplaced = m_ECS.EmplaceComponent<MeshComponent>(
			m_GUIEntity,
			m_PreviousMeshTransform,
			meshHandle.Other,
			DX::DX11::ShaderSetType::CIRCLE
		);

		m_Logger.LogFatalNLTaggedIf(
			!emplaced,
			FuncTag,
			L"Failed to emplace mesh component on GUI entity."
		);

		m_PreviousCameraTransform = Common::RigidTransform::Translate(
			Common::Float3(0.0f, 0.0f, -10.0f)
		);

		emplaced = m_ECS.EmplaceComponent<CameraComponent>(
			m_CameraEntity,
			m_PreviousCameraTransform
		);

		m_Logger.LogFatalNLTaggedIf(
			!emplaced,
			FuncTag,
			L"Failed to emplace camera component on camera entity."
		);

		Common::Rect renderArea = m_Renderer.CurrentArea();

		CameraData cameraData(
			m_PreviousCameraTransform,
			PerspectiveParams(45.0f),
			0.5f,
			100.0f
		);

		m_Renderer.SetCamera(cameraData);
		m_Renderer.SetShaderSet(DX::DX11::ShaderSetType::CIRCLE);

		const Asset::MeshDescriptor& meshDescriptor = meshData.Descriptor;

		m_DrawDescriptor.TotalVertices = meshDescriptor.VertexCount;
		m_DrawDescriptor.VertexByteStride = meshDescriptor.VertexByteStride;
		m_DrawDescriptor.TotalIndices = meshDescriptor.IndexCount;
		m_DrawDescriptor.IndexByteStride = meshDescriptor.IndexByteStride;

		m_DrawDescriptor.TotalInstances = 1;
		m_DrawDescriptor.InstanceByteStride = sizeof(m_InstanceRadius);
		m_DrawDescriptor.IndicesPerInstance = m_DrawDescriptor.TotalIndices;
	}

	void EditorLayer::OnDetach() noexcept
	{
		MeshComponent* pGuiMeshComponent = m_ECS.GetComponent<MeshComponent>(m_GUIEntity);

		if (pGuiMeshComponent != nullptr)
			m_AssetManager.Unregister(pGuiMeshComponent->Handle);

		m_ECS.DestroyEntity(m_CameraEntity);
		m_ECS.DestroyEntity(m_GUIEntity);
	}

	void EditorLayer::OnUpdate(float deltaTime) noexcept
	{
		constexpr std::wstring_view FuncTag = L"EditorLayer [OnUpdate] | ";

		CameraComponent* pCameraComponent = m_ECS.GetComponent<CameraComponent>(m_CameraEntity);
		MeshComponent* pMeshComponent = m_ECS.GetComponent<MeshComponent>(m_GUIEntity);

		m_Logger.LogFatalNLTaggedIf(
			pMeshComponent == nullptr,
			FuncTag,
			L"Failed to retrieve GUI entity's mesh component."
		);

		/* Here so visual studio doesn't yell at me for de-referencing a
		 * null pointer even though fatal logs terminate the program. */
		if (pMeshComponent == nullptr)
			return;

		Asset::AMResultPair<const Asset::Mesh*> meshResult = m_AssetManager.GetMesh(pMeshComponent->Handle);
		const Asset::Mesh* pMesh = meshResult.Other;

		m_Logger.LogFatalNLTaggedIf(
			meshResult.Result.Failed(),
			FuncTag,
			L"Failed to retrieve GUI entity's mesh."
		);

		/* Here so visual studio doesn't yell at me for de-referencing a
		 * null pointer even though fatal logs terminate the program. */
		if (pMesh == nullptr)
			return;

		Common::Transform& currentMeshTransform = pMeshComponent->Transform;
		Common::RigidTransform& currentCameraTransform = pCameraComponent->RigidTransform;

		if (!currentCameraTransform.IsNearEqual(m_PreviousCameraTransform))
			m_Renderer.SetCameraTransform(currentCameraTransform);

		/* Note to future self: 
		 * 
		 *   Since the orthographic projection matrix is based
		 *	   off of screen-space units (pixels), all vertices
		 *     must be in screen-space units as well; not world-space. */
		/*Common::Float2 vertices[] = {
			{ 0.0f,   0.0f },
			{ 100.0f, 0.0f },
			{ 100.0,  100.0f },
			{ 0.0f,   100.0f }
		};*/

		m_Renderer.CacheModelTransform(currentMeshTransform);

		m_Renderer.DrawIndexedInstanced(
			pMesh->Data.VertexData,
			pMesh->Data.IndexData,
			Common::Utility::AsBytes(m_InstanceRadius),
			m_DrawDescriptor
		);

		ShowCameraWindow(currentCameraTransform);
		ShowMeshWindow(currentMeshTransform);
	}

	void EditorLayer::ShowMeshWindow(Common::Transform& meshTransform) noexcept
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
		if (m_Renderer.ImGuiCollapsingHeader("Instance"))
		{
			// Begin a child region inside the collapsing header
			m_Renderer.ImGuiBeginChild("ChildInstance", childSize, ImGuiChildFlags_Border);

			m_Renderer.ImGuiSlider("Radius", &m_InstanceRadius, 0, 10);

			m_Renderer.ImGuiEndChild();
		}

		m_Renderer.ImGuiEnd();
	}

	void EditorLayer::ShowCameraWindow(Common::RigidTransform& cameraTransform) noexcept
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