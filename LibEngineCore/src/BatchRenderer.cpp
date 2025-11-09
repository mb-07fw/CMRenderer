#include "PCH.hpp"
#include "BatchRenderer.hpp"
#include "Types.hpp"

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
			gltfBasicVertexId,
			std::span<const InputElement>(Elements.data(), Elements.size())
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
			/* Note: This is essentially throwing away copying work done for any previous meshes,
			 *		   not the best long term strategy. 
			 */
			m_Vertices.clear();
			m_Indices.clear();

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

	void BatchRenderer::SubmitMesh(ECS::Entity e) noexcept
	{
		auto mesh = m_ECS.TryGetComponent<MeshComponent>(e);
		auto material = m_ECS.TryGetComponent<MaterialComponent>(e);

		if (mesh.Null() || material.Null())
			return;
		else if (m_MeshMetadata.find(mesh->ID) != m_MeshMetadata.end())
			return;

		m_SubmittedMeshes.emplace_back(e);
		m_MeshSubmitted = true;
	}

	void BatchRenderer::SubmitInstance(
		Asset::AssetID meshID,
		Asset::AssetID materialID,
		ECS::Entity e
	) noexcept
	{
		m_Batches[Key(meshID, materialID)].Instances.emplace_back(e);
	}

	void BatchRenderer::CollectMeshes() noexcept
	{
		/* Ensure all homogenous entries (Mesh and Material id's) are grouped together
		 * to minimize state changes through differing materials, and so that all mesh data can be
		 * pooled in the order that the entities are contained. */
		std::sort(
			m_SubmittedMeshes.begin(),
			m_SubmittedMeshes.end(),
			[&](ECS::Entity a, ECS::Entity b)
			{
				auto meshA = m_ECS.TryGetComponent<MeshComponent>(a);
				auto meshB = m_ECS.TryGetComponent<MeshComponent>(b);

				auto materialA = m_ECS.TryGetComponent<MaterialComponent>(a);
				auto materialB = m_ECS.TryGetComponent<MaterialComponent>(b);

				if (meshA.Null() || materialA.Null() ||
					meshB.Null() || materialB.Null())
					return false;

				/* Priority ordering is | Mesh ID | Material ID |
				 * ex) a = (0, 0)
				 *	   b = (0, 1)
				 *	   c = (0, 0) 
				 *	   d = (1, 0) 
				 *     e = (0, 2)
				 * 
				 * Final order: a, c, b, e, d */
				if (meshA->ID != meshB->ID)
					return meshA->ID < meshB->ID;
				else
					return materialA->ID < materialB->ID;
			}
		);

		size_t totalVertices = 0;
		size_t totalIndices = 0;

		/* First iteration to get total element size of vertex and index buffers... */
		for (ECS::Entity e : m_SubmittedMeshes)
		{
			auto mesh = m_ECS.GetComponent<MeshComponent>(e);

			/* Mesh data is already present, no need to copy again. */
			if (m_MeshMetadata.find(mesh.ID) != m_MeshMetadata.end())
				continue;

			ConstView<Asset::Mesh> meshAsset;
			m_AssetManager.GetMesh(mesh.ID, meshAsset);

			CM_ENGINE_ASSERT(meshAsset.NonNull());

			totalVertices += meshAsset->Data.Vertices.size();
			totalIndices += meshAsset->Data.Indices.size();
		}

		/* Resize to ensure elements are valid (size is reflected). */
		m_Vertices.resize(totalVertices);
		m_Indices.resize(totalIndices);

		size_t currentVertexOffset = 0;
		size_t currentIndexOffset = 0;

		/* Second iteration to copy all data... */
		for (ECS::Entity e : m_SubmittedMeshes)
		{
			auto mesh = m_ECS.GetComponent<MeshComponent>(e);
			auto material = m_ECS.GetComponent<MaterialComponent>(e);

			/* Mesh data is already present, no need to copy again. */
			if (m_MeshMetadata.find(mesh.ID) != m_MeshMetadata.end())
				continue;

			ConstView<Asset::Mesh> meshAsset;
			m_AssetManager.GetMesh(mesh.ID, meshAsset);

			CM_ENGINE_ASSERT(meshAsset.NonNull());

			const std::vector<Asset::Vertex>& vertices = meshAsset->Data.Vertices;
			const std::vector<Asset::Index>& indices = meshAsset->Data.Indices;

			size_t verticesSizeBytes = sizeof(Asset::Vertex) * vertices.size();
			size_t indicesSizeBytes = sizeof(Asset::Index) * indices.size();

			std::memcpy(
				&m_Vertices[currentVertexOffset],
				vertices.data(),
				verticesSizeBytes
			);

			std::memcpy(
				&m_Indices[currentIndexOffset],
				indices.data(),
				indicesSizeBytes
			);

			MeshMeta& metadata = m_MeshMetadata[mesh.ID];

			metadata.MeshID = mesh.ID;
			metadata.MaterialID = material.ID;
			metadata.OffsetVertices = (int32_t)currentVertexOffset;
			metadata.OffsetIndices = (uint32_t)currentIndexOffset;
			metadata.NumVertices = (uint32_t)vertices.size();
			metadata.NumIndices = (uint32_t)indices.size();

			currentVertexOffset += vertices.size();
			currentIndexOffset += indices.size();
		}

		m_MeshSubmitted = false;
	}

	void BatchRenderer::Flush() noexcept
	{
		ShaderID basicVSID = m_Graphics.GetShader(L"Gltf_Basic_VS");
		ShaderID basicPSID = m_Graphics.GetShader(L"Gltf_Basic_PS");

		constexpr uint32_t OffsetBytes = 0;
		constexpr uint32_t StartIndex = 0;

		m_Graphics.BindVertexBuffer(m_VB_Vertices, sizeof(Asset::Vertex), OffsetBytes, S_VB_Vertices_Register);
		m_Graphics.BindVertexBuffer(m_VB_Instances, sizeof(BatchInstance), OffsetBytes, S_VB_Instances_Register);
		m_Graphics.BindIndexBuffer(m_IB_Indices, DataFormat::UInt16, StartIndex);

		m_Graphics.BindInputLayout(m_IL_Basic);

		Asset::AssetID lastMaterialID;
		for (const auto& [meshID, metadata] : m_MeshMetadata)
		{
			ConstView<Asset::Material> material;
			m_AssetManager.GetMaterial(metadata.MaterialID, material);

			CM_ENGINE_ASSERT(material.NonNull());

			/* TODO: Do other stuff if entity has a TextureComponent... */
			if (metadata.MaterialID != lastMaterialID)
			{
				m_Graphics.SetBuffer(m_CB_Material, &material->Data, sizeof(material->Data));
				m_Graphics.BindConstantBufferPS(m_CB_Material, S_CB_Material_Register);
			}

			lastMaterialID = metadata.MaterialID;

			/* TODO: Speciallized shaders need to be used depending of textures, lights, etc... */
			if (m_Graphics.LastVS() != basicVSID)
				m_Graphics.BindShader(basicVSID);
			if (m_Graphics.LastPS() != basicPSID)
				m_Graphics.BindShader(basicPSID);

			Batch& batch = m_Batches[Key(metadata.MeshID, metadata.MaterialID)];

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