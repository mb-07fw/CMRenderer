#pragma once

#include "Scene/Node.hpp"

namespace CMEngine::Scene
{
	class SceneGraph
	{
	public:
		SceneGraph() noexcept;
		~SceneGraph() = default;

		SceneGraph(const SceneGraph&) = delete;
		SceneGraph& operator=(const SceneGraph&) = delete;
		SceneGraph(SceneGraph&&) = default;
		SceneGraph& operator=(SceneGraph&&) = default;
	public:
		void AddNode(Node::NodeType type, ECS::Entity e) noexcept;

		inline [[nodiscard]] const Node::RootNode& Root() const noexcept { return m_Root; }
	private:
		Node::RootNode m_Root;
	};

	using SceneID = size_t;

	class Scene
	{
	public:
		Scene() noexcept;
		~Scene() = default;

		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;
		Scene(Scene&&) = default;
		Scene& operator=(Scene&&) = default;
	public:
		void OnActivate() noexcept;
		void OnDeactivate() noexcept;

		inline [[nodiscard]] SceneGraph& Graph() noexcept { return m_Graph; }
		inline [[nodiscard]] const SceneGraph& Graph() const noexcept { return m_Graph; }
	private:
		SceneGraph m_Graph;
	};
}