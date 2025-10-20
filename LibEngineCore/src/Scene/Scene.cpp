#include "PCH.hpp"
#include "Scene/Scene.hpp"

namespace CMEngine::Scene
{
    SceneGraph::SceneGraph() noexcept
        : m_Root()
    {
    }

    void SceneGraph::AddNode(Node::NodeType type, ECS::Entity e) noexcept
    {
        m_Root.Nodes.emplace_back(type, e);
    }

    Scene::Scene() noexcept
    {
    }

    void Scene::OnActivate() noexcept
    {
    }

    void Scene::OnDeactivate() noexcept
    {
    }
}