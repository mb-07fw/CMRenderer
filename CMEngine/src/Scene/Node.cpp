#include "PCH.hpp"
#include "Scene/Node.hpp"

namespace CMEngine::Scene::Node
{
	NodeBasic::NodeBasic(PrimitveType primitiveType, DerivedType type, const std::string& name) noexcept
		: m_PrimitiveType(primitiveType),
		  m_Type(type),
		  m_Name(name)
	{
	}

	BranchNode::BranchNode(DerivedType type, const std::string& name) noexcept
		: NodeBasic(PrimitveType::Branch, type, name)
	{
	}

	LeafNode::LeafNode(DerivedType type, const std::string& name) noexcept
		: NodeBasic(PrimitveType::Leaf, type, name)
	{
	}

	RootNode::RootNode() noexcept
	{
	}

	Camera3D::Camera3D(const CameraData& data) noexcept
		: BranchNode(DerivedType::Camera3D, "Camera3D"),
		  m_Camera(data)
	{
	}
}