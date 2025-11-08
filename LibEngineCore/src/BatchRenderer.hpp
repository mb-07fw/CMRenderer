#pragma once

#include "Graphics.hpp"
#include "Component.hpp"
#include "Math.hpp"
#include "Types.hpp"
#include "ECS/ECS.hpp"
#include "Asset/AssetManager.hpp"

#include <vector>
#include <unordered_map>
#include <functional> // std::hash

namespace CMEngine::Renderer
{
	struct Key
	{
		inline constexpr Key(
			Asset::AssetID meshID,
			Asset::AssetID materialID
		) noexcept
			: MeshID(meshID),
			  MaterialID(materialID)
		{
		}

		[[nodiscard]] bool operator==(const Key&) const = default;

		Asset::AssetID MeshID;
		Asset::AssetID MaterialID;
	};

	struct KeyHasher
	{
		size_t operator()(const Key& key) const noexcept
		{
			size_t hash = std::hash<Asset::AssetID>{}(key.MeshID);
			HashCombine(hash, std::hash<Asset::AssetID>{}(key.MaterialID));

			return hash;
		}
	};

	struct MeshMeta
	{
		Asset::AssetID MeshID;
		Asset::AssetID MaterialID;
		int32_t OffsetVertices = 0; /* int32_t for conformity with ID3D11DeviceContext::DrawIndexedInstanced's baseVertexLocation. */
		uint32_t OffsetIndices = 0; 
		uint32_t NumVertices = 0;
		uint32_t NumIndices = 0;
	};

	struct BatchInstance
	{
		inline BatchInstance(const Math::Mat4& transform) noexcept
			: Transform(transform)
		{
		}

		BatchInstance(const BatchInstance&) = default;
		BatchInstance(BatchInstance&&) = default;
		BatchInstance& operator=(const BatchInstance&) = default;
		BatchInstance& operator=(BatchInstance&&) = default;

		Math::Mat4 Transform;
	};

	struct Batch
	{
		uint32_t NumInstances = 0;
		uint32_t OffsetInstances = 0;
		std::vector<ECS::Entity> Instances; // ECS::Entity's with TransformComponent's.
	};

	class BatchRenderer
	{
		friend class Renderer;
	public:
		BatchRenderer(ECS::ECS& ecs, AGraphics& graphics, Asset::AssetManager& assetManager) noexcept;
		~BatchRenderer() noexcept = default;
	public:
		void BeginBatch() noexcept;
		void EndBatch() noexcept;

		void SubmitMesh(ECS::Entity e) noexcept;

		void SubmitInstance(
			Asset::AssetID meshID,
			Asset::AssetID materialID,
			ECS::Entity e
		) noexcept;
	private:
		void CollectMeshes() noexcept;

		void Flush() noexcept;

		void StoreMesh(MeshComponent mesh, MaterialComponent material) noexcept;
	private:
		static constexpr uint32_t S_VB_Vertices_Register = 0;
		static constexpr uint32_t S_VB_Instances_Register = 1;
		static constexpr uint32_t S_CB_Material_Register = 0;
		ECS::ECS& m_ECS;
		AGraphics& m_Graphics;
		Asset::AssetManager& m_AssetManager;
		std::vector<Asset::Vertex> m_Vertices;
		std::vector<Asset::Index> m_Indices;
		std::vector<BatchInstance> m_Instances;
		std::vector<ECS::Entity> m_SubmittedMeshes;
		std::unordered_map<Asset::AssetID, MeshMeta> m_MeshMetadata;
		std::unordered_map<Key, Batch, KeyHasher> m_Batches;
		Resource<IInputLayout> m_IL_Basic;
		Resource<IBuffer> m_VB_Vertices;
		Resource<IBuffer> m_VB_Instances;
		Resource<IBuffer> m_IB_Indices;
		Resource<IBuffer> m_CB_Material;
		bool m_MeshSubmitted = false;
	};
}