#pragma once

#include "ECS/Entity.hpp"
#include "Component.hpp"

#include <cstdint>
#include <vector>
#include <string_view>

namespace CMEngine::Scene::Node
{
	enum class NodeType : int8_t
	{
		Invalid = -1,
		Camera3D
	};

	struct Node
	{
		inline Node(NodeType type, ECS::Entity e) noexcept
			: Type(type),
			  Entity(e)
		{
		}

		NodeType Type = NodeType::Invalid;
		ECS::Entity Entity;
	};

	struct RootNode
	{
		std::vector<Node> Nodes;
	};

	struct IDerivedNode : public Node
	{
		using Node::Node;

		inline static constexpr [[nodiscard]] bool IsLeaf(NodeType type) noexcept;
		inline static constexpr [[nodiscard]] bool IsBranch(NodeType type) noexcept;

		inline static constexpr [[nodiscard]] std::string_view NameOf(NodeType type) noexcept;

		virtual [[nodiscard]] bool IsLeaf() const noexcept = 0;
		virtual [[nodiscard]] bool IsBranch() const noexcept = 0;
	};

	struct Camera3D : public IDerivedNode
	{
		inline Camera3D(ECS::Entity e, CameraComponent& camera) noexcept
			: IDerivedNode(NodeType::Camera3D, e),
			  m_Camera(camera)
		{
		}

		[[nodiscard]] Float3 GetOrigin() const noexcept;
		[[nodiscard]] void UpdateOrigin(Float3 newOrigin) noexcept;

		virtual [[nodiscard]] bool IsLeaf() const noexcept override { return false; }
		virtual [[nodiscard]] bool IsBranch() const noexcept override { return true; }

		CameraComponent& m_Camera;
	};

	inline constexpr [[nodiscard]] bool IDerivedNode::IsLeaf(NodeType type) noexcept
	{
		switch (type)
		{
		case NodeType::Invalid: [[fallthrough]];
		default:
			return false;
		case NodeType::Camera3D:
			return false;
		}
	}

	inline constexpr [[nodiscard]] bool IDerivedNode::IsBranch(NodeType type) noexcept
	{
		switch (type)
		{
		case NodeType::Invalid:  [[fallthrough]]; 
		default:				 return false;
		case NodeType::Camera3D: return true;
		}
	}

	inline constexpr [[nodiscard]] std::string_view IDerivedNode::NameOf(NodeType type) noexcept
	{
		switch (type)
		{
		case NodeType::Invalid:  [[fallthrough]];

		default:				 return { "Invalid" };
		case NodeType::Camera3D: return { "Camera3D" };
		}
	}
}