#include "PCH.hpp"
#include "Layer.hpp"
#include "Component.hpp"

namespace CMEngine
{
	EditorLayer::EditorLayer(
		APlatform& platform,
		Asset::AssetManager& assetManager,
		Scene::SceneManager& sceneManager,
		ECS::ECS& ecs
	) noexcept
		: m_Platform(platform),
		  m_AssetManager(assetManager),
		  m_SceneManager(sceneManager),
		  m_ECS(ecs)
	{
	}

	void EditorLayer::OnAttach() noexcept
	{
		AGraphics& graphics = m_Platform.GetGraphics();
		AWindow& window = m_Platform.GetWindow();
		
		//mP_CameraCB = graphics.CreateBuffer(GPUBufferType::Constant, GPUBufferFlag::Dynamic);

		CameraData cameraData;
		cameraData.Origin = Float3(0.0f, 0.0f, -10.0f);
		cameraData.Aspect = window.ClientResolution().Aspect();
		cameraData.FovAngle = 45.0f;
		cameraData.NearZ = 0.5f;
		cameraData.FarZ = 100.0f;

		m_EditorScene = m_SceneManager.NewScene(cameraData);

		//graphics.SetBuffer(mP_CameraCB, &camera.Matrices, sizeof(camera.Matrices));
		//graphics.BindConstantBufferVS(mP_CameraCB, 0);

		spdlog::info("(EditorLayer) Attatched.");
	}

	void EditorLayer::OnDetatch() noexcept
	{
		spdlog::info("(EditorLayer) Detatched.");
	}

	void EditorLayer::OnUpdate() noexcept
	{
		//CameraComponent* pCamera = m_ECS.GetComponent<CameraComponent>(m_Camera);

		AGraphics& graphics = m_Platform.GetGraphics();
		graphics.StartFrame(Color4::Black());

		m_SceneManager.DisplaySceneGraph();

		//Float3 previousPos = pCamera->Data.Origin;

		//if (pCamera)
		//{
		//	if (ImGui::Begin("EditorLayer"))
		//	{
		//		if (ImGui::CollapsingHeader("Camera"))
		//			ImGui::SliderFloat3("Pos", pCamera->Data.Origin.Underlying(), -20.0f, 20.0f);
		//	}

		//	ImGui::End();

		//	/* Recreate the view matrix of the camera if it's origin changed. */
		//	if (!pCamera->Data.Origin.IsNearEqual(previousPos))
		//	{
		//		pCamera->CreateViewMatrix();
		//		graphics.SetBuffer(mP_CameraCB, &pCamera->Matrices, sizeof(pCamera->Matrices));
		//	}
		//}
		//else
		//	spdlog::critical("(EditorLayer) [OnUpdate] Internal error: Failed to retrieve current CameraComponent.");
		
		/* Drawing done here...  */

		graphics.EndFrame();
	}
}