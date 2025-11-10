#pragma once

#include "Graphics.hpp"
#include "Component.hpp"
#include "Math.hpp"
#include "Types.hpp"
#include "ECS/ECS.hpp"
#include "Asset/AssetManager.hpp"

#include <vector>
#include <map>
#include <functional> // std::hash

namespace CMEngine::Renderer
{
	struct Key
	{
		inline constexpr Key(
			ECS::Entity e, /* For optionally retrieving Resource<ITexture>... */
			Asset::AssetID meshID,
			Asset::AssetID materialID,
			Asset::AssetID textureID = Asset::AssetID()
		) noexcept
			: Entity(e),
			  MeshID(meshID),
			  MaterialID(materialID),
			  TextureID(textureID)
		{
		}

		inline [[nodiscard]] bool operator==(const Key& other) const noexcept
		{
			return MeshID == other.MeshID &&
				MaterialID == other.MaterialID &&
				TextureID == other.TextureID;
		}

		/* Strict weak ordering... */
		inline [[nodiscard]] bool operator<(const Key& rhs) const noexcept
		{
			if (MeshID != rhs.MeshID)
				return MeshID < rhs.MeshID;
			if (MaterialID != rhs.MaterialID)
				return MaterialID < rhs.MaterialID;
			return TextureID < rhs.TextureID;
		}

		/* TODO: Move to Batch as Representative (first entity added) */
		ECS::Entity Entity; /* Not sorted, we don't care about any key's entity. */
		Asset::AssetID MeshID;
		Asset::AssetID MaterialID;
		Asset::AssetID TextureID;
	};

	struct MeshMeta
	{
		Asset::AssetID MeshID;
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
		/* ECS::Entity's with TransformComponent's, MaterialComponent's, TextureComponent's, etc. */
		std::vector<ECS::Entity> Instances;
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

		void SubmitMesh(MeshComponent mesh) noexcept;

		void SubmitInstance(
			ECS::Entity e,
			Asset::AssetID meshID,
			Asset::AssetID materialID,
			Asset::AssetID textureID = Asset::AssetID()
		) noexcept;
	private:
		void CollectMeshes() noexcept;

		void Flush() noexcept;
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
		std::vector<MeshComponent> m_SubmittedMeshes;
		std::unordered_map<Asset::AssetID, MeshMeta> m_MeshMetadata;
		/* Ensure Batch's are sorted based on their mesh, material, and/or texture id's. (Olog(n))... */
		std::map<Key, Batch> m_Batches;
		Resource<IInputLayout> m_IL_Basic;
		Resource<IBuffer> m_VB_Vertices;
		Resource<IBuffer> m_VB_Instances;
		Resource<IBuffer> m_IB_Indices;
		Resource<IBuffer> m_CB_Material;
		bool m_MeshSubmitted = false;
	};
}