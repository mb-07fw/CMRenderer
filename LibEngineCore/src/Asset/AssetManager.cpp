#include "PCH.hpp"
#include "Macros.hpp"
#include "Asset/AssetManager.hpp"
#include "Log.hpp"

namespace CMEngine::Asset
{
	class ModelImporterImpl
	{
	public:
		ModelImporterImpl() = default;
		~ModelImporterImpl() = default;

		void LoadMesh(Mesh& mesh, ConstView<aiMesh> aiMesh, ConstView<aiScene> scene) noexcept;

		void LoadVertices(Mesh& mesh, ConstView<aiMesh> aiMesh) noexcept;
		void LoadIndices(Mesh& mesh, ConstView<aiMesh> aiMesh) noexcept;

		Assimp::Importer Importer;
	};

	void ModelImporterImpl::LoadMesh(Mesh& mesh, ConstView<aiMesh> aiMesh, ConstView<aiScene> scene) noexcept
	{
		LoadVertices(mesh, aiMesh);
		LoadIndices(mesh, aiMesh);
	}

	void ModelImporterImpl::LoadVertices(Mesh& mesh, ConstView<aiMesh> aiMesh) noexcept
	{
		mesh.Data.Vertices.reserve(aiMesh->mNumVertices);

		for (unsigned int i = 0; i < aiMesh->mNumVertices; ++i)
		{
			aiVector3D texCoord = aiMesh->HasTextureCoords(mesh.Index)
				? aiMesh->mTextureCoords[0][i]  // UV channel 0
				: aiVector3D(0.0f, 0.0f, 0.0f); // fallback

			aiVector3D& vertex = aiMesh->mVertices[i];
			aiVector3D& normal = aiMesh->mNormals[i];

			Float3 vertexF3(vertex.x, vertex.y, vertex.z);
			Float3 normalF3(normal.x, normal.y, normal.z);
			Float3 texCoordF3(texCoord.x, texCoord.y, texCoord.z);

			mesh.Data.Vertices.emplace_back(vertexF3, normalF3, texCoordF3);
		}
	}

	void ModelImporterImpl::LoadIndices(Mesh& mesh, ConstView<aiMesh> aiMesh) noexcept
	{
		/* Since we triangulated, each face should be a triangle (i.e., require 3 indices. ex. 0, 1, 2). */
		UINT numIndices = aiMesh->mNumFaces * 3;
		mesh.Data.Indices.reserve(numIndices);

		for (unsigned int i = 0; i < aiMesh->mNumFaces; ++i)
		{
			const auto& face = aiMesh->mFaces[i];
			CM_ENGINE_ASSERT(face.mNumIndices == 3);

			mesh.Data.Indices.emplace_back((uint16_t)(face.mIndices[0]));
			mesh.Data.Indices.emplace_back((uint16_t)(face.mIndices[1]));
			mesh.Data.Indices.emplace_back((uint16_t)(face.mIndices[2]));
		}
	}

	AssetManager::AssetManager() noexcept
		: mP_ModelImporter(std::make_unique<ModelImporterImpl>())
	{
 	   Init();
	}

	AssetManager::~AssetManager() noexcept
	{
 	   Shutdown();
	}

	void AssetManager::Init() noexcept
	{
	}

	void AssetManager::Shutdown() noexcept
	{
	}

	Result AssetManager::LoadModel(const std::filesystem::path& modelPath, AssetID& outModelID) noexcept
	{
		if (!std::filesystem::exists(modelPath))
		{
			spdlog::warn(
				"(AssetManager) Internal warning: Provided model path doesn't exist. Path: {}",
				modelPath.generic_string()
			);

			return ResultType::Failed_File_Absent;
		}

		ConstView<aiScene> scene = mP_ModelImporter->Importer.ReadFile(
			modelPath.generic_string(),
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_ConvertToLeftHanded
		);

		if (!scene)
		{
			CM_ENGINE_LOG_WARN(
				"(AssetManager) Internal warning: Error occured loading model. "
				"Error: {}", mP_ModelImporter->Importer.GetErrorString()
			);

			return ResultType::Failed_File_Import;
		}

		outModelID = AssetID::Registered(AssetType::Model, NextGlobalID());
		Model& model = m_ModelMap[outModelID];

		model.Meshes.reserve(scene->mNumMeshes);
		model.Materials.reserve(scene->mNumMaterials);

		for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
		{
			ConstView<aiMesh> aiMesh = scene->mMeshes[meshIndex];

			if (!aiMesh)
			{
				CM_ENGINE_LOG_WARN(
					"(AssetManager) Internal warning: Failed to retrieve mesh at index: {}. File: {}",
					meshIndex, modelPath.generic_string()
				);

				continue;
			}

			AssetID meshID = AssetID::Registered(AssetType::Mesh, NextGlobalID());

			model.Meshes.emplace_back(meshID);

			Mesh& mesh = m_MeshMap[meshID];
			mesh.ModelID = outModelID;
			mesh.Index = meshIndex;

			mP_ModelImporter->LoadMesh(mesh, aiMesh, scene);
		}

		for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex)
		{
			AssetID materialID = AssetID::Registered(AssetType::Material, NextGlobalID());
			model.Materials.emplace_back(materialID);

			Material& material = m_MaterialMap[materialID];
			MaterialData& materialData = material.Data;

			ConstView<aiMaterial> pMaterial = scene->mMaterials[materialIndex];

			/* TODO: handle material.TextureID */
			material.ModelID = outModelID;
			material.Index = materialIndex;

			aiColor4D baseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
			aiGetMaterialColor(pMaterial, AI_MATKEY_BASE_COLOR, &baseColor);
			materialData.BaseColor = { baseColor.r, baseColor.g, baseColor.b, baseColor.a };

			float metallic = 0.0f;
			aiGetMaterialFloat(pMaterial, AI_MATKEY_METALLIC_FACTOR, &metallic);
			materialData.Metallic = metallic;

			float roughness = 0.0f;
			aiGetMaterialFloat(pMaterial, AI_MATKEY_ROUGHNESS_FACTOR, &roughness);
			materialData.Roughness = roughness;
		}

		CM_ENGINE_LOG_INFO(
			"(AssetManager) Internal info: Successfully loaded model. Name: {}, GlobalID: {}", 
			modelPath.generic_string(), outModelID.GlobalID()
		);

		return ResultType::Succeeded;
	}

	Result AssetManager::GetModel(AssetID id, ConstView<Model>& outModel) noexcept
	{
		using MapTy = decltype(m_ModelMap);
		return GetAsset<MapTy, Model>(m_ModelMap, AssetType::Model, id, outModel);
	}

	Result AssetManager::GetMesh(AssetID id, ConstView<Mesh>& outMesh) noexcept
	{
		using MapTy = decltype(m_MeshMap);
		return GetAsset<MapTy, Mesh>(m_MeshMap, AssetType::Mesh, id, outMesh);
	}

	Result AssetManager::GetMaterial(AssetID id, ConstView<Material>& outMaterial) noexcept
	{
		using MapTy = decltype(m_MaterialMap);
		return GetAsset<MapTy, Material>(m_MaterialMap, AssetType::Material, id, outMaterial);
	}

	bool AssetManager::Unregister(AssetID& outID) noexcept
	{
		if (!outID.IsRegistered())
			return false;

		CleanupID(outID);
		return true;
	}

	[[nodiscard]] bool AssetManager::IsMapped(AssetID id) noexcept
	{
		switch (id.Type())
		{
		case AssetType::Mesh:
			return m_MeshMap.contains(id);
		case AssetType::Invalid: [[fallthrough]];
		case AssetType::Material: [[fallthrough]];
		case AssetType::Texture: [[fallthrough]];
		default:
			return false;
		}
	}

	[[nodiscard]] uint32_t AssetManager::NextGlobalID() noexcept
	{
		if (m_FreeGlobalIDs.empty())
			return m_TotalAssetCount++;

		uint32_t next = m_FreeGlobalIDs.back();
		m_FreeGlobalIDs.pop_back();

		return next;
	}

	void AssetManager::CleanupID(AssetID& outHandle) noexcept
	{
		if (!outHandle.IsRegistered() || outHandle.Type() == AssetType::Invalid)
			return;

		outHandle.SetRegistered(false);

		switch (outHandle.Type())
		{
		case AssetType::Mesh:
			if (auto it = m_MeshMap.find(outHandle); it != m_MeshMap.end())
				m_MeshMap.erase(it);

			break;
		case AssetType::Material: /* TODO: Implement later... */
		case AssetType::Texture: /* TODO: Implement later... */
			break;
		default:
			return;
		}

		m_FreeGlobalIDs.emplace_back(outHandle.GlobalID());
	}
}