#pragma once

#include "PlatformUtil.hpp"
#include "Scene/Node.hpp"
#include "ECS/ECS.hpp"
#include "Component.hpp"

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
		template <Node::DerivedNode Ty, typename... Args>
		inline Node::NodePtr<Ty> EmplaceNode(Args&&... args) noexcept;

		inline void Cleanup() noexcept { return m_Root.DestroyChildren(); }
		inline [[nodiscard]] const Node::RootNode& Root() const noexcept { return m_Root; }
	private:
		Node::RootNode m_Root;
	};

	template <Node::DerivedNode Ty, typename... Args>
	inline Node::NodePtr<Ty> SceneGraph::EmplaceNode(Args&&... args) noexcept
	{
		return m_Root.EmplaceNode<Ty>(std::forward<Args>(args)...);
	}

	using SceneID = size_t;

	class Scene
	{
	public:
		Scene(const CameraData& mainCamera) noexcept;
		~Scene() = default;

		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;
		Scene(Scene&&) = default;
		Scene& operator=(Scene&&) = default;
	public:
		void OnActivate() noexcept;
		void OnDeactivate() noexcept;

		inline [[nodiscard]] const SceneGraph& Graph() const noexcept { return m_Graph; }
		inline [[nodiscard]] const Node::RootNode& RootNode() const noexcept { return m_Graph.Root(); }
	private:
		SceneGraph m_Graph;
	};
}