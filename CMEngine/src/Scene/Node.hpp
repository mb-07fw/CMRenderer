#pragma once

#include "Component.hpp"
#include "Types.hpp"

#include <vector>
#include <string>
#include <memory>
#include <type_traits>

#include <cstdint>

namespace CMEngine::Scene::Node
{
#pragma region Basic Nodes
	enum class PrimitveType : int8_t
	{
		Invalid = -1,
		Root,
		Branch,
		Leaf,
	};

	enum class DerivedType : int8_t
	{
		Invalid = -1,
		Camera3D
	};

	class INode
	{
	public:
		INode() = default;
		virtual ~INode() = default;
	public:
		virtual [[nodiscard]] PrimitveType PrimitiveType() const noexcept = 0;
		virtual [[nodiscard]] DerivedType Type() const noexcept = 0;
		virtual [[nodiscard]] const std::string& Name() const noexcept = 0;
		virtual [[nodiscard]] bool IsRoot() const noexcept = 0;
		virtual [[nodiscard]] bool IsBranch() const noexcept = 0;
		virtual [[nodiscard]] bool IsLeaf() const noexcept = 0;
	};

	class NodeBasic : public INode
	{
	public:
		NodeBasic(PrimitveType primitiveType, DerivedType type, const std::string& name) noexcept;
		virtual ~NodeBasic() = default;
	public:
		inline virtual [[nodiscard]] PrimitveType PrimitiveType() const noexcept override { return m_PrimitiveType; }
		inline virtual [[nodiscard]] DerivedType Type() const noexcept override { return m_Type; }
		inline virtual [[nodiscard]] const std::string& Name() const noexcept override { return m_Name; }

		inline virtual [[nodiscard]] bool IsRoot() const noexcept override { return m_PrimitiveType == PrimitveType::Root; }
		inline virtual [[nodiscard]] bool IsBranch() const noexcept override { return m_PrimitiveType == PrimitveType::Branch; }
		inline virtual [[nodiscard]] bool IsLeaf() const noexcept override { return m_PrimitiveType == PrimitveType::Leaf; }
	protected:
		const PrimitveType m_PrimitiveType;
		const DerivedType m_Type;
		const std::string m_Name;
	};

	template <typename Ty>
	concept DerivedNode = std::is_base_of_v<INode, Ty>;

	template <DerivedNode Ty = INode>
	using NodePtr = std::shared_ptr<Ty>;

	class BranchNode : public NodeBasic
	{
	public:
		BranchNode(DerivedType type, const std::string& name) noexcept;
		~BranchNode() = default;

		BranchNode(const BranchNode&) = delete;
		BranchNode& operator=(const BranchNode&) = delete;
		BranchNode(BranchNode&&) = default;
		BranchNode& operator=(BranchNode&&) = default;
	private:
		std::vector<NodePtr<>> m_Children;
		const INode* mP_Parent = nullptr;
	};

	class LeafNode : public NodeBasic
	{
	public:
		LeafNode(DerivedType type, const std::string& name) noexcept;
		~LeafNode() = default;
	private:
		const INode* mP_Parent = nullptr;
	};

	class RootNode
	{
	public:
		RootNode() noexcept;
		~RootNode() = default;

		RootNode(const RootNode&) = delete;
		RootNode& operator=(const RootNode&) = delete;
		RootNode(RootNode&&) = default;
		RootNode& operator=(RootNode&&) = default;
	public:
		template <DerivedNode Ty, typename... Args>
		inline NodePtr<Ty> EmplaceNode(Args&&... args) noexcept;

		inline void DestroyChildren() noexcept { m_Nodes.clear(); }

		inline [[nodiscard]] const std::vector<NodePtr<>>& Nodes() const noexcept { return m_Nodes; }
	private:
		std::vector<NodePtr<>> m_Nodes;
	};

	template <DerivedNode Ty, typename... Args>
	inline NodePtr<Ty> RootNode::EmplaceNode(Args&&... args) noexcept
	{
		auto temp = std::make_shared<Ty>(std::forward<Args>(args)...);
		m_Nodes.emplace_back(temp);
		return temp;
	}
#pragma endregion

#pragma region Specialized Nodes
	class Camera3D : public BranchNode
	{
	public:
		Camera3D(const CameraData& data) noexcept;
		~Camera3D() = default;
	private:
		CameraComponent m_Camera;
	};
#pragma endregion
}