#include "PCH.hpp"
#include "Scene/Scene.hpp"

namespace CMEngine::Scene
{
    SceneGraph::SceneGraph() noexcept
        : m_Root()
    {
    }

    Scene::Scene(const CameraData& mainCamera) noexcept
    {
        m_Graph.EmplaceNode<Node::Camera3D>(mainCamera);
    }

    void Scene::OnActivate() noexcept
    {
    }

    void Scene::OnDeactivate() noexcept
    {
        m_Graph.Cleanup();
    }
}