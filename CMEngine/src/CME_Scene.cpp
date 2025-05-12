#include "CME_PCH.hpp"
#include "CME_Scene.hpp"

namespace CMEngine
{
	ICMScene::ICMScene(
		CMCommon::CMLoggerWide& engineLoggerRef,
		CMCommon::CMECS& ECSRef,
		CMRenderer::CMRenderer& rendererRef,
		std::function<void(float)> onUpdateFunc
	) noexcept
		: m_EngineLoggerRef(engineLoggerRef),
		  m_ECSRef(ECSRef),
		  m_RendererRef(rendererRef),
		  m_OnUpdateFunc(onUpdateFunc)
	{
	}

	void ICMScene::OnUpdate(float deltaTime) noexcept
	{
		m_OnUpdateFunc(deltaTime);
	}

	CMTestScene::CMTestScene(
		CMCommon::CMLoggerWide& engineLoggerRef,
		CMCommon::CMECS& ECSRef,
		CMRenderer::CMRenderer& rendererRef
	) noexcept
		: ICMStockScene(engineLoggerRef, ECSRef, rendererRef, [this](float deltaTime) { OnUpdate(deltaTime); })
	{
	}

	void CMTestScene::OnStart() noexcept
	{
		m_CameraEntity = m_ECSRef.CreateEntity();

		m_EngineLoggerRef.LogFatalNLIf(
			!m_ECSRef.EmplaceComponent<CMRigidTransformComponent>(
				m_CameraEntity,
				CMCommon::CMRigidTransform(
					CMCommon::CMFloat3(),
					CMCommon::CMFloat3(0.0f, 0.0f, -10.0f)
				)
			),
			L"CMTestScene [OnStart] | Failed to emplace transform on camera entity."
		);

		m_MeshEntity = m_ECSRef.CreateEntity();

		/* Quad vertices...
		std::vector<float> vertices = {
			-1.0f,  1.0f,
			 1.0f,  1.0f,
			 1.0f, -1.0f,
			-1.0f, -1.0f
		};

		* Cube vertices (duplicated)...
		std::vector<float> vertices = {
			// Front face
			-1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,

			// Back face
			-1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			// Left face
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f, -1.0f,

			// Right face
			 1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f, -1.0f,

			// Top face
			-1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,

			// Bottom face
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f
		};

		* Cube indices (duplicated)...
		std::vector<uint16_t> indices = {
			0, 1, 2,
			0, 2, 3,

			6, 5, 4,
			7, 6, 4,

			10, 9, 8,
			11, 10, 8,

			12, 13, 14,
			12, 14, 15,

			16, 17, 18,
			16, 18, 19,

			22, 21, 20,
			23, 22, 20
		};*/

		std::vector<float> vertices = {
			-1.0f, 0.0f, -1.0f,
			 1.0f, 0.0f, -1.0f,
			 1.0f, 0.0f,  1.0f,
			-1.0f, 0.0f,  1.0f
		};

		std::vector<uint16_t> indices = {
			2, 1, 0,
			3, 2, 0
		};

		m_EngineLoggerRef.LogFatalNLIf(
			!m_ECSRef.EmplaceComponent<CMMeshComponent>(
				m_MeshEntity, 
				CMCommon::CMTransform(
					CMCommon::CMFloat3(10.0f, 1.0f, 10.0f),
					CMCommon::CMFloat3(),
					CMCommon::CMFloat3(0.0f, -2.5f, 0.0f)
				),
				vertices,
				indices,
				CMRenderer::CMDirectX::DXShaderSetType::CMCUBE
			),
			L"CMTestScene [OnStart] | Failed to emplace transform on camera entity."
		);

		CMCommon::CMRigidTransform rigidTransform;
		m_RendererRef.RenderContext().SetCamera(CMRenderer::CMCameraData(rigidTransform, 45.0f, 0.5f, 100.0f));
	}

	void CMTestScene::OnEnd() noexcept
	{
		m_EngineLoggerRef.LogFatalNLIf(
			!(m_ECSRef.DestroyEntity(m_CameraEntity) || m_ECSRef.DestroyEntity(m_MeshEntity)),
			L"CMTestScene [OnEnd] | Failed to destroy entities."
		);
	}

	void CMTestScene::OnUpdate(float deltaTime) noexcept
	{
		m_EngineLoggerRef.LogFatalNLIf(!m_RendererRef.IsInitialized(), L"CMTestScene [OnUpdate] | Renderer isn't initialized.");

		CMMeshComponent* pMeshComponent = m_ECSRef.GetComponent<CMMeshComponent>(m_MeshEntity);

		m_EngineLoggerRef.LogFatalNLIf(pMeshComponent == nullptr, L"CMTestScene [OnUpdate] | Failed to retrieve mesh component.");

		CMRigidTransformComponent* pCameraTransformComponent = m_ECSRef.GetComponent<CMRigidTransformComponent>(m_CameraEntity);

		m_EngineLoggerRef.LogFatalNLIf(pCameraTransformComponent == nullptr, L"CMTestScene [OnUpdate] | Failed to retrieve camera transform component.");

		// Here so Intelisense doesn't yell at me for de-referencing a null pointer, even though fatal logs terminate the program.
		if (pMeshComponent == nullptr || pCameraTransformComponent == nullptr)
			return;

		// Update model matrix if transform changed.
		if (!pMeshComponent->Transform.IsNearEqual(m_PreviousMeshTransform))
		{
			m_RendererRef.RenderContext().SetModelMatrix(
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

		if (m_RendererRef.RenderContext().CurrentShaderSet() != pMeshComponent->SetType)
			m_RendererRef.RenderContext().SetShaderSet(pMeshComponent->SetType);

		CMRenderer::CMDirectX::DXContext& renderContextRef = m_RendererRef.RenderContext();

		// Update camera if transform changed.
		if (!pCameraTransformComponent->IsNearEqual(m_PreviousCameraTransform))
		{
			renderContextRef.SetCameraTransform(pCameraTransformComponent->RigidTransform);
			m_PreviousCameraTransform = pCameraTransformComponent->RigidTransform;
		}

		renderContextRef.DrawIndexed(pMeshComponent->Vertices, pMeshComponent->Indices, sizeof(float) * 3);

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
		CMRenderer::CMDirectX::DXContext& renderContextRef = m_RendererRef.RenderContext();

		// Full width, fixed height (scrollable if content overflows)
		ImVec2 childSize(0.0f, 150.0f);

		renderContextRef.ImGuiBegin("Mesh Transform");

		if (renderContextRef.ImGuiCollapsingHeader("Scaling"))
		{
			// Begin a child region inside the collapsing header
			renderContextRef.ImGuiBeginChild("ChildScaling", childSize, ImGuiChildFlags_Border);

			renderContextRef.ImGuiSlider("X", &meshTransform.Scaling.x, 0.01f, 20.0f);
			renderContextRef.ImGuiSlider("Y", &meshTransform.Scaling.y, 0.01f, 20.0f);
			renderContextRef.ImGuiSlider("Z", &meshTransform.Scaling.z, 0.01f, 20.0f);

			renderContextRef.ImGuiEndChild();
		}
		if (renderContextRef.ImGuiCollapsingHeader("Rotation"))
		{
			// Begin a child region inside the collapsing header
			renderContextRef.ImGuiBeginChild("ChildRotation", childSize, ImGuiChildFlags_Border);

			renderContextRef.ImGuiSliderAngle("X", &meshTransform.Rotation.x, -360.0f, 360.0f);
			renderContextRef.ImGuiSliderAngle("Y", &meshTransform.Rotation.y, -360.0f, 360.0f);
			renderContextRef.ImGuiSliderAngle("Z", &meshTransform.Rotation.z, -360.0f, 360.0f);

			renderContextRef.ImGuiEndChild();
		}
		if (renderContextRef.ImGuiCollapsingHeader("Translation"))
		{
			// Begin a child region inside the collapsing header
			renderContextRef.ImGuiBeginChild("ChildTranslation", childSize, ImGuiChildFlags_Border);

			renderContextRef.ImGuiSlider("X", &meshTransform.Translation.x, -20, 20);
			renderContextRef.ImGuiSlider("Y", &meshTransform.Translation.y, -20, 20);
			renderContextRef.ImGuiSlider("Z", &meshTransform.Translation.z, -20, 20);

			renderContextRef.ImGuiEndChild();
		}

		renderContextRef.ImGuiEnd();
	}

	void CMTestScene::ShowCameraWindow(CMCommon::CMRigidTransform& cameraTransform) noexcept
	{
		CMRenderer::CMDirectX::DXContext& renderContextRef = m_RendererRef.RenderContext();

		// Full width, fixed height (scrollable if content overflows)
		ImVec2 childSize(0.0f, 150.0f);

		renderContextRef.ImGuiBegin("Camera Transform");

		if (renderContextRef.ImGuiCollapsingHeader("Rotation"))
		{
			// Begin a child region inside the collapsing header
			renderContextRef.ImGuiBeginChild("ChildRotation", childSize, ImGuiChildFlags_Border);

			renderContextRef.ImGuiSliderAngle("X", &cameraTransform.Rotation.x, -360.0f, 360.0f);
			renderContextRef.ImGuiSliderAngle("Y", &cameraTransform.Rotation.y, -360.0f, 360.0f);
			//renderContextRef.ImGuiSliderAngle("Z", &cameraTransform.Rotation.z, -360.0f, 360.0f);

			renderContextRef.ImGuiEndChild();
		}
		if (renderContextRef.ImGuiCollapsingHeader("Translation"))
		{
			// Begin a child region inside the collapsing header
			renderContextRef.ImGuiBeginChild("ChildTranslation", childSize, ImGuiChildFlags_Border);

			renderContextRef.ImGuiSlider("X", &cameraTransform.Translation.x, -20, 20);
			renderContextRef.ImGuiSlider("Y", &cameraTransform.Translation.y, -20, 20);
			renderContextRef.ImGuiSlider("Z", &cameraTransform.Translation.z, -20, 20);

			renderContextRef.ImGuiEndChild();
		}

		renderContextRef.ImGuiEnd();
	}
}