#include "PCH.hpp"
#include "Macros.hpp"
#include "Asset/AssetManager.hpp"

namespace CMEngine::Asset
{
	class ModelImporterImpl
	{
	public:
		ModelImporterImpl() = default;
		~ModelImporterImpl() = default;

		Assimp::Importer Importer;
	};

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

	[[nodiscard]] Result AssetManager::LoadMesh(const std::filesystem::path& modelPath, AssetID& outID) noexcept
	{
		if (!std::filesystem::exists(modelPath))
		{
			spdlog::warn(
				"(AssetManager) Internal warning: Provided model name doesn't exist in model directory. Model: {}",
				modelPath.generic_string()
			);

			return ResultType::FAILED_FILE_ABSENT;
		}

		const aiScene* pModel = mP_ModelImporter->Importer.ReadFile(
			ENGINE_CORE_RESOURCES_MODEL_DIRECTORY "/cube.gltf",
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_ConvertToLeftHanded
		);

		if (!pModel || !pModel->mMeshes[0])
		{
			spdlog::warn(
				"(AssetManager) Internal warning: Error occured loading model. "
				"Error: {}", mP_ModelImporter->Importer.GetErrorString()
			);

			return ResultType::FAILED;
		}

		const aiMesh* pMesh = pModel->mMeshes[0];

		outID = AssetID::Registered(AssetType::MESH, NextGlobalID());
		uint32_t globalID = outID.GlobalID();

		if (auto it = m_MeshMap.find(globalID); it != m_MeshMap.end())
			spdlog::warn("(AssetManager) Internal warning: Mesh data is already present at global id: {}. Data will be overwritten.", globalID);

		Mesh& mesh = m_MeshMap[globalID];
		mesh.Data.Vertices.reserve(pMesh->mNumVertices);

		for (unsigned int i = 0; i < pMesh->mNumVertices; ++i)
		{
			aiVector3D texCoord = pMesh->HasTextureCoords(0)
				? pMesh->mTextureCoords[0][i]  // UV channel 0
				: aiVector3D(0.0f, 0.0f, 0.0f); // fallback

			aiVector3D& vertex = pMesh->mVertices[i];
			aiVector3D& normal = pMesh->mNormals[i];

			Float3 vertexF3(vertex.x, vertex.y, vertex.z);
			Float3 normalF3(normal.x, normal.y, normal.z);
			Float3 texCoordF3(texCoord.x, texCoord.y, texCoord.z);

			mesh.Data.Vertices.emplace_back(vertexF3, normalF3, texCoordF3);
		}

		/* Since we triangulated, each face should be a triangle (i.e., require 3 indices. ex. 0, 1, 2). */
		UINT numIndices = pMesh->mNumFaces * 3;
		mesh.Data.Indices.reserve(numIndices);

		for (unsigned int i = 0; i < pMesh->mNumFaces; ++i)
		{
			const auto& face = pMesh->mFaces[i];
			CM_ENGINE_ASSERT(face.mNumIndices == 3);

			mesh.Data.Indices.emplace_back(static_cast<uint16_t>(face.mIndices[0]));
			mesh.Data.Indices.emplace_back(static_cast<uint16_t>(face.mIndices[1]));
			mesh.Data.Indices.emplace_back(static_cast<uint16_t>(face.mIndices[2]));
		}

		const aiMaterial* pMaterial = pModel->mMaterials[pMesh->mMaterialIndex];
		Material material = {};

		aiColor4D baseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		if (AI_SUCCESS == aiGetMaterialColor(pMaterial, AI_MATKEY_BASE_COLOR, &baseColor))
			material.BaseColor = { baseColor.r, baseColor.g, baseColor.b, baseColor.a };

		float metallic = 0.0f;
		if (AI_SUCCESS == aiGetMaterialFloat(pMaterial, AI_MATKEY_METALLIC_FACTOR, &metallic))
			material.Metallic = metallic;

		float roughness = 0.0f;
		if (AI_SUCCESS == aiGetMaterialFloat(pMaterial, AI_MATKEY_ROUGHNESS_FACTOR, &roughness))
			material.Roughness = roughness;

		mesh.Data.Material = std::move(material);

		spdlog::info("(AssetManager) Internal info: Successfully loaded model. Name: {}, GlobalID: {}", modelPath.generic_string(), globalID);
		return ResultType::SUCCEEDED;
	}

	[[nodiscard]] Result AssetManager::GetMesh(AssetID id, ConstView<Mesh>& pOutMesh) noexcept
	{
		pOutMesh = nullptr;

		if (id.Type() != AssetType::MESH)
			return ResultType::FAILED_HANDLE_MISMATCHING_ASSET_TYPE;
		else if (!id.IsRegistered())
			return ResultType::FAILED_HANDLE_NOT_REGISTERED;

		auto it = m_MeshMap.find(id.GlobalID());
		if (it == m_MeshMap.end())
			return ResultType::FAILED_HANDLE_NOT_MAPPED;

		pOutMesh = &it->second;
		return ResultType::SUCCEEDED;
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
		case AssetType::MESH:
			return m_MeshMap.contains(id.GlobalID());
		case AssetType::INVALID: [[fallthrough]];
		case AssetType::TEXTURE: [[fallthrough]];
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
		if (!outHandle.IsRegistered() || outHandle.Type() == AssetType::INVALID)
			return;

		outHandle.SetRegistered(false);

		switch (outHandle.Type())
		{
		case AssetType::MESH:
			if (auto it = m_MeshMap.find(outHandle.GlobalID()); it != m_MeshMap.end())
				m_MeshMap.erase(it);

			break;
		case AssetType::TEXTURE:
			break;
		default:
			return;
		}

		m_FreeGlobalIDs.emplace_back(outHandle.GlobalID());
	}
}