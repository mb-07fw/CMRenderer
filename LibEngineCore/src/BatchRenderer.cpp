#include "PCH.hpp"
#include "BatchRenderer.hpp"
#include "Types.hpp"
#include "Log.hpp"

namespace CMEngine::Renderer
{
	BatchRenderer::BatchRenderer(ECS::ECS& ecs, AGraphics& graphics, Asset::AssetManager& assetManager) noexcept
		: m_ECS(ecs),
		  m_Graphics(graphics),
		  m_AssetManager(assetManager)
	{
		/* approx. 10 kb of vertices... */
		constexpr size_t InitialVertexBufferSize = (1024 * 10) / sizeof(Asset::Vertex);
		constexpr size_t InitialIndexBufferSize = (1024) / sizeof(Asset::Index);

		m_Vertices.reserve(InitialVertexBufferSize);
		m_Indices.reserve(InitialIndexBufferSize);

		m_VB_Vertices  = m_Graphics.CreateBuffer(GPUBufferType::Vertex);
		m_VB_Instances = m_Graphics.CreateBuffer(GPUBufferType::Vertex, GPUBufferFlag::Dynamic);
		m_IB_Indices   = m_Graphics.CreateBuffer(GPUBufferType::Index);
		m_CB_Material  = m_Graphics.CreateBuffer(GPUBufferType::Constant, GPUBufferFlag::Dynamic);

		constexpr std::array<InputElement, 4> Elements = {
			InputElement(
				"POSITION",
				0, // Semantic index
				DataFormat::Float32x3,
				0, // Input slot (vb slot to read data from)
				0, // Aligned byte offset
				InputClass::PerVertex,
				0 // Instance step rate
			),
			InputElement(
				"NORMAL",
				0,
				DataFormat::Float32x3,
				0,
				G_InputElement_InferByteOffset,
				InputClass::PerVertex,
				0
			),
			InputElement(
				"TEXCOORD",
				0,
				DataFormat::Float32x3,
				0,
				G_InputElement_InferByteOffset,
				InputClass::PerVertex,
				0
			),
			InputElement(
				"INST_TRANSFORM",
				G_InputElement_ExpandAsMultiple,
				DataFormat::Mat4,
				1,
				G_InputElement_InferByteOffset,
				InputClass::PerInstance,
				1
			)
		};

		ShaderID gltfBasicVertexId = m_Graphics.GetShader(L"Gltf_Basic_VS");
		m_IL_Basic = m_Graphics.CreateInputLayout(
			std::span<const InputElement>(Elements.data(), Elements.size()),
			gltfBasicVertexId
		);
	}

	void BatchRenderer::BeginBatch() noexcept
	{
		/* Clear previous per-frame instances. */
		for (auto& [key, batch] : m_Batches)
		{
			batch.OffsetInstances = 0;
			batch.NumInstances = 0;
			batch.Instances.clear();
		}

		m_Instances.clear();
	}

	void BatchRenderer::EndBatch() noexcept
	{
		if (m_MeshSubmitted)
		{
			CollectMeshes();

			m_Graphics.SetBuffer(m_VB_Vertices, m_Vertices.data(), m_Vertices.size() * sizeof(Asset::Vertex));
			m_Graphics.SetBuffer(m_IB_Indices, m_Indices.data(), m_Indices.size() * sizeof(Asset::Index));
		}

		/* First iteration to get total number of instances (potentially save allocations). */
		size_t totalInstances = 0;
		for (const auto& [key, batch] : m_Batches)
			totalInstances += batch.Instances.size();
		
		m_Instances.reserve(totalInstances);

		/* Consolidate all instances into a single buffer... */
		size_t currentInstanceOffset = 0;
		for (auto& [key, batch] : m_Batches)
		{
			batch.OffsetInstances = (uint32_t)currentInstanceOffset;
			batch.NumInstances = (uint32_t)batch.Instances.size();

			ConstView<ECS::Storage<TransformComponent>> storage = m_ECS.GetStorage<TransformComponent>();
			for (ECS::Entity e : batch.Instances)
				if (const TransformComponent* pTransform = storage->GetComponent(e); pTransform)
					m_Instances.emplace_back(pTransform->ModelMatrix);
			
			currentInstanceOffset += batch.Instances.size();
		}

		m_Graphics.SetBuffer(m_VB_Instances, m_Instances.data(), m_Instances.size() * sizeof(BatchInstance));
	}

	void BatchRenderer::SubmitMesh(MeshComponent mesh) noexcept
	{
		if (m_MeshMetadata.find(mesh.ID) != m_MeshMetadata.end())
			return;

		m_SubmittedMeshes.emplace_back(mesh);
		m_MeshSubmitted = true;
	}

	void BatchRenderer::SubmitInstance(
		ECS::Entity e,
		Asset::AssetID meshID,
		Asset::AssetID materialID,
		Asset::AssetID textureID
	) noexcept
	{
		if (!meshID || !materialID)
			return;

		m_Batches[Key(e, meshID, materialID, textureID)].Instances.emplace_back(e);
	}

	void BatchRenderer::CollectMeshes() noexcept
	{
		size_t totalVertices = 0;
		size_t totalIndices = 0;

		/* Remove all submitted meshes with an invalid mesh id... */
		std::erase_if(
			m_SubmittedMeshes,
			[&](const MeshComponent& mesh)
			{
				ConstView<Asset::Mesh> meshAsset;
				m_AssetManager.GetMesh(mesh.ID, meshAsset);

				/* No associated mesh asset...  */
				if (meshAsset.Null())
					return true;

				/* Accumulate total buffer sizes while meshes are retrieved... */
				totalVertices += meshAsset->Data.Vertices.size();
				totalIndices += meshAsset->Data.Indices.size();

				return false;
			}
		);

		m_Vertices.resize(totalVertices);
		m_Indices.resize(totalIndices);

		uint32_t offsetVertices = 0;
		uint32_t offsetIndices = 0;

		uint32_t currentOffsetVertices = 0;
		uint32_t currentOffsetIndices = 0;

		for (const MeshComponent& mesh : m_SubmittedMeshes)
		{
			ConstView<Asset::Mesh> meshAsset;
			m_AssetManager.GetMesh(mesh.ID, meshAsset);

			/* Null checks aren't necessary because of previous filtering... */
			const auto& vertices = meshAsset->Data.Vertices;
			const auto& indices = meshAsset->Data.Indices;

			currentOffsetVertices = offsetVertices;
			currentOffsetIndices = offsetIndices;

			offsetVertices += (uint32_t)vertices.size();
			offsetIndices += (uint32_t)indices.size();

			auto it = m_MeshMetadata.find(mesh.ID);
			bool metadataExists = it != m_MeshMetadata.end();

			bool verticesRequireCopy = !metadataExists ||
				it->second.OffsetVertices != (int32_t)currentOffsetVertices;

			bool indicesRequireCopy = !metadataExists ||
				it->second.OffsetIndices != currentOffsetIndices;

			if (!verticesRequireCopy &&
				!indicesRequireCopy)
				continue;

			if (verticesRequireCopy)
				std::memcpy(
					std::to_address(m_Vertices.begin() + currentOffsetVertices),
					vertices.data(),
					sizeof(Asset::Vertex) * vertices.size()
				);

			if (indicesRequireCopy)
				std::memcpy(
					std::to_address(m_Indices.begin() + currentOffsetIndices),
					indices.data(),
					sizeof(Asset::Index) * indices.size()
				);

			/* Mesh data was present, but was re-copied due to layout change. */
			if (metadataExists)
			{
				it->second.OffsetVertices = currentOffsetVertices;
				it->second.OffsetIndices = currentOffsetIndices;
				continue;
			}

			MeshMeta& metadata = m_MeshMetadata[mesh.ID];
			metadata.MeshID = mesh.ID;
			metadata.OffsetVertices = currentOffsetVertices;
			metadata.OffsetIndices = currentOffsetIndices;
			metadata.NumVertices = (uint32_t)vertices.size();
			metadata.NumIndices = (uint32_t)indices.size();
		}

		m_MeshSubmitted = false;
	}

	void BatchRenderer::Flush() noexcept
	{
		ShaderID basicVSID = m_Graphics.GetShader(L"Gltf_Basic_VS");
		ShaderID basicPSID = m_Graphics.GetShader(L"Gltf_Basic_PS");
		ShaderID texturePSID = m_Graphics.GetShader(L"Gltf_Texture_PS");

		constexpr uint32_t OffsetBytes = 0;
		constexpr uint32_t StartIndex = 0;

		m_Graphics.BindVertexBuffer(m_VB_Vertices, sizeof(Asset::Vertex), OffsetBytes, S_VB_Vertices_Register);
		m_Graphics.BindVertexBuffer(m_VB_Instances, sizeof(BatchInstance), OffsetBytes, S_VB_Instances_Register);
		m_Graphics.BindIndexBuffer(m_IB_Indices, DataFormat::UInt16, StartIndex);

		m_Graphics.BindInputLayout(m_IL_Basic);

		m_Graphics.BindShader(basicVSID);
		m_Graphics.BindShader(basicPSID);

		Asset::AssetID lastMaterialID;
		Asset::AssetID lastTextureID;

		for (const auto& [key, batch] : m_Batches)
		{
			ConstView<Asset::Material> material;
			m_AssetManager.GetMaterial(key.MaterialID, material);

			CM_ENGINE_ASSERT(material.NonNull());

			if (key.MaterialID != lastMaterialID)
			{
				m_Graphics.SetBuffer(m_CB_Material, &material->Data, sizeof(material->Data));
				m_Graphics.BindConstantBufferPS(m_CB_Material, S_CB_Material_Register);
			}

			bool wasTextureDependent = lastTextureID.IsRegistered();
			bool isTextureDependent = key.TextureID.IsRegistered();
			bool differentTextureID = key.TextureID != lastTextureID;

			lastMaterialID = key.MaterialID;
			lastTextureID = key.TextureID;

			auto texture = m_ECS.TryGetComponent<TextureComponent>(key.Entity);

			/* Current texture use is different from previous.. */
			if (isTextureDependent != wasTextureDependent)
				m_Graphics.BindShader(isTextureDependent ? texturePSID : basicPSID);


			if (texture.NonNull() &&
				isTextureDependent &&
				differentTextureID)
				m_Graphics.BindTexture(texture->Texture);

			auto it = m_MeshMetadata.find(key.MeshID);
			CM_ENGINE_ASSERT(it != m_MeshMetadata.end());

			MeshMeta& metadata = it->second;

			m_Graphics.DrawIndexedInstanced(
				metadata.NumIndices,
				batch.NumInstances,
				metadata.OffsetIndices,
				metadata.OffsetVertices,
				batch.OffsetInstances
			);
		}
	}
}