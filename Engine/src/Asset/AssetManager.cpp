#include "Asset/AssetManager.hpp"

#include <fstream>
#include <system_error>

//#include "PCH.hpp"
//#include "Macros.hpp"
//#include "Log.hpp"

namespace Engine::Asset
{
	AssetManager::AssetManager() noexcept
	{
	}

	AssetManager::~AssetManager() noexcept
	{
	}

	[[nodiscard]] AssetID AssetManager::LoadShader(
		const std::filesystem::path& filePath,
		View<Result> optOutResult
	) noexcept
	{
		if (!std::filesystem::exists(filePath))
		{
			SetResultIfValid(optOutResult, ResultType::Failed_File_Absent);
			return AssetID();
		}
		else if (filePath.extension() != ".hlsl")
		{
			SetResultIfValid(optOutResult, ResultType::Failed_File_Import_Invalid_Format);
			return AssetID();
		}

		std::vector<char> fileBuffer;

		std::error_code ec;
		size_t fileSizeBytes = std::filesystem::file_size(filePath, ec);

		if (ec)
		{
			SetResultIfValid(optOutResult, ResultType::Failed_File_Import);
			return AssetID();
		}

		fileBuffer.resize(fileSizeBytes);

		std::ifstream stream(filePath, std::ios::binary);
		if (!stream)
		{
			SetResultIfValid(optOutResult, ResultType::Failed_File_Import);
			return AssetID();
		}

		stream.read(fileBuffer.data(), fileBuffer.size());

		if (!stream)
		{
			SetResultIfValid(optOutResult, ResultType::Failed_File_Import);
			return AssetID();
		}

		AssetID id = AssetID(AssetType::Shader, NextGlobalID());
		id.SetRegistered(true);

		Shader& shader = m_ShaderMap[id];
		shader.ID = id;
		shader.RawData = std::move(fileBuffer);

		SetResultIfValid(optOutResult, ResultType::Succeeded);
		return id;
	}
	
	[[nodiscard]] ConstView<Shader> AssetManager::GetShader(AssetID id) const noexcept
	{
		return GetAsset<Shader>(m_ShaderMap, AssetType::Shader, id);
	}

	bool AssetManager::UnregisterID(AssetID& outID) noexcept
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
		case AssetType::Mesh: [[fallthrough]];
			//return m_MeshMap.contains(id);
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
			/*if (auto it = m_MeshMap.find(outHandle); it != m_MeshMap.end())
				m_MeshMap.erase(it);

			break;*/
		case AssetType::Material: /* TODO: Implement later... */
		case AssetType::Texture: /* TODO: Implement later... */
			break;
		default:
			return;
		}

		m_FreeGlobalIDs.emplace_back(outHandle.GlobalID());
	}

	void AssetManager::SetResultIfValid(View<Result> result, ResultType type) noexcept
	{
		if (result.NonNull())
			result->Type = type;
	}

	//class ModelImporterImpl
	//{
	//public:
	//	ModelImporterImpl() = default;
	//	~ModelImporterImpl() = default;

	//	void LoadMesh(Mesh& mesh, ConstView<aiMesh> aiMesh, ConstView<aiScene> scene) noexcept;

	//	void LoadVertices(Mesh& mesh, ConstView<aiMesh> aiMesh) noexcept;
	//	void LoadIndices(Mesh& mesh, ConstView<aiMesh> aiMesh) noexcept;

	//	Assimp::Importer Importer;
	//};

	//void ModelImporterImpl::LoadMesh(Mesh& mesh, ConstView<aiMesh> aiMesh, ConstView<aiScene> scene) noexcept
	//{
	//	LoadVertices(mesh, aiMesh);
	//	LoadIndices(mesh, aiMesh);
	//}

	//void ModelImporterImpl::LoadVertices(Mesh& mesh, ConstView<aiMesh> aiMesh) noexcept
	//{
	//	mesh.Data.Vertices.reserve(aiMesh->mNumVertices);

	//	for (unsigned int i = 0; i < aiMesh->mNumVertices; ++i)
	//	{
	//		aiVector3D texCoord = aiMesh->HasTextureCoords(mesh.Index)
	//			? aiMesh->mTextureCoords[0][i]  // UV channel 0
	//			: aiVector3D(0.0f, 0.0f, 0.0f); // fallback

	//		aiVector3D& vertex = aiMesh->mVertices[i];
	//		aiVector3D& normal = aiMesh->mNormals[i];

	//		Float3 vertexF3(vertex.x, vertex.y, vertex.z);
	//		Float3 normalF3(normal.x, normal.y, normal.z);
	//		Float2 texCoordF2(texCoord.x, texCoord.y);

	//		mesh.Data.Vertices.emplace_back(vertexF3, normalF3, texCoordF2);
	//	}
	//}

	//void ModelImporterImpl::LoadIndices(Mesh& mesh, ConstView<aiMesh> aiMesh) noexcept
	//{
	//	/* Since we triangulated, each face should be a triangle (i.e., require 3 indices. ex. 0, 1, 2). */
	//	UINT numIndices = aiMesh->mNumFaces * 3;
	//	mesh.Data.Indices.reserve(numIndices);

	//	for (unsigned int i = 0; i < aiMesh->mNumFaces; ++i)
	//	{
	//		const auto& face = aiMesh->mFaces[i];
	//		CM_ENGINE_ASSERT(face.mNumIndices == 3);

	//		mesh.Data.Indices.emplace_back((uint16_t)(face.mIndices[0]));
	//		mesh.Data.Indices.emplace_back((uint16_t)(face.mIndices[1]));
	//		mesh.Data.Indices.emplace_back((uint16_t)(face.mIndices[2]));
	//	}
	//}

//	AssetManager::AssetManager() noexcept
//		: mP_ModelImporter(std::make_unique<ModelImporterImpl>())
//	{
// 	   Init();
//	}
//
//	AssetManager::~AssetManager() noexcept
//	{
// 	   Shutdown();
//	}
//
//	void AssetManager::Init() noexcept
//	{
//	}
//
//	void AssetManager::Shutdown() noexcept
//	{
//	}
//
//	Result AssetManager::LoadModel(const std::filesystem::path& modelPath, AssetID& outModelID) noexcept
//	{
//		if (!std::filesystem::exists(modelPath))
//		{
//			CM_ENGINE_LOG_WARN(
//				"(AssetManager) Internal warning: Provided model path doesn't exist. Path: {}",
//				modelPath.generic_string()
//			);
//
//			return ResultType::Failed_File_Absent;
//		}
//
//		ConstView<aiScene> scene = mP_ModelImporter->Importer.ReadFile(
//			modelPath.generic_string(),
//			aiProcess_Triangulate |
//			aiProcess_JoinIdenticalVertices |
//			aiProcess_ConvertToLeftHanded
//		);
//
//		if (!scene)
//		{
//			CM_ENGINE_LOG_WARN(
//				"(AssetManager) Internal warning: Error occured loading model. "
//				"Error: {}", mP_ModelImporter->Importer.GetErrorString()
//			);
//
//			return ResultType::Failed_File_Import;
//		}
//
//		outModelID = AssetID::Registered(AssetType::Model, NextGlobalID());
//		Model& model = m_ModelMap[outModelID];
//		File& file = m_LoadedFiles[outModelID];
//		file.Path = modelPath;
//
//		model.Meshes.reserve(scene->mNumMeshes);
//		model.Materials.reserve(scene->mNumMaterials);
//
//		for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
//		{
//			ConstView<aiMesh> aiMesh = scene->mMeshes[meshIndex];
//
//			if (!aiMesh)
//			{
//				CM_ENGINE_LOG_WARN(
//					"(AssetManager) Internal warning: Failed to retrieve mesh at index: {}. File: {}",
//					meshIndex, modelPath.generic_string()
//				);
//
//				continue;
//			}
//
//			AssetID meshID = AssetID::Registered(AssetType::Mesh, NextGlobalID());
//
//			model.Meshes.emplace_back(meshID);
//
//			Mesh& mesh = m_MeshMap[meshID];
//			mesh.ModelID = outModelID;
//			mesh.Index = meshIndex;
//
//			mP_ModelImporter->LoadMesh(mesh, aiMesh, scene);
//		}
//
//		for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex)
//		{
//			AssetID materialID = AssetID::Registered(AssetType::Material, NextGlobalID());
//			model.Materials.emplace_back(materialID);
//
//			Material& material = m_MaterialMap[materialID];
//			MaterialData& materialData = material.Data;
//
//			ConstView<aiMaterial> pMaterial = scene->mMaterials[materialIndex];
//
//			/* TODO: handle material.TextureID */
//			material.ModelID = outModelID;
//			material.Index = materialIndex;
//
//			aiColor4D baseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
//			aiGetMaterialColor(pMaterial, AI_MATKEY_BASE_COLOR, &baseColor);
//			materialData.BaseColor = { baseColor.r, baseColor.g, baseColor.b, baseColor.a };
//
//			float metallic = 0.0f;
//			aiGetMaterialFloat(pMaterial, AI_MATKEY_METALLIC_FACTOR, &metallic);
//			materialData.Metallic = metallic;
//
//			float roughness = 0.0f;
//			aiGetMaterialFloat(pMaterial, AI_MATKEY_ROUGHNESS_FACTOR, &roughness);
//			materialData.Roughness = roughness;
//		}
//
//		CM_ENGINE_LOG_INFO(
//			"(AssetManager) Internal info: Successfully loaded model. Name: {}, GlobalID: {}", 
//			modelPath.generic_string(), outModelID.GlobalID()
//		);
//
//		return ResultType::Succeeded;
//	}
//
//	Result AssetManager::LoadTexture(const std::filesystem::path& modelPath, AssetID& outTextureID) noexcept
//	{
//		if (!std::filesystem::exists(modelPath))
//		{
//			CM_ENGINE_LOG_WARN(
//				"(AssetManager) Internal warning: Provided texture path doesn't exist. Path: {}",
//				modelPath.generic_string()
//			);
//
//			return ResultType::Failed_File_Absent;
//		}
//
//		std::ifstream stream(modelPath, std::ios::binary);
//
//		if (!stream.is_open())
//		{
//			CM_ENGINE_LOG_WARN(
//				"(AssetManager) Internal warning: Failed to open texture file. Path: {}",
//				modelPath.generic_string()
//			);
//
//			return ResultType::Failed_File_Import;
//		}
//
//		size_t byteSize = std::filesystem::file_size(modelPath);
//
//		if (byteSize == 0)
//		{
//			CM_ENGINE_LOG_WARN(
//				"(AssetManager) Internal warning: Texture file is empty. (byte size is 0) Path: {}",
//				modelPath.generic_string()
//			);
//
//			return ResultType::Failed_File_Import;
//		}
//
//		outTextureID = AssetID::Registered(AssetType::Texture, NextGlobalID());
//		Texture& texture = m_TextureMap[outTextureID];
//
//		texture.pBuffer = std::move(std::unique_ptr<std::byte>(new std::byte[byteSize]));
//		texture.SizeBytes = byteSize;
//
//		stream.read(
//			reinterpret_cast<char*>(texture.pBuffer.get()),
//			static_cast<std::streamsize>(texture.SizeBytes)
//		);
//
//		return ResultType::Succeeded;
//	}
//
//	Result AssetManager::GetModel(AssetID id, ConstView<Model>& outModel) noexcept
//	{
//		using MapTy = decltype(m_ModelMap);
//		return GetAsset<MapTy, Model>(m_ModelMap, AssetType::Model, id, outModel);
//	}
//
//	Result AssetManager::GetMesh(AssetID id, ConstView<Mesh>& outMesh) noexcept
//	{
//		using MapTy = decltype(m_MeshMap);
//		return GetAsset<MapTy, Mesh>(m_MeshMap, AssetType::Mesh, id, outMesh);
//	}
//
//	Result AssetManager::GetMaterial(AssetID id, ConstView<Material>& outMaterial) noexcept
//	{
//		using MapTy = decltype(m_MaterialMap);
//		return GetAsset<MapTy, Material>(m_MaterialMap, AssetType::Material, id, outMaterial);
//	}
//
//	Result AssetManager::GetTexture(AssetID id, ConstView<Texture>& outTexture) noexcept
//	{
//		using MapTy = decltype(m_TextureMap);
//		return GetAsset<MapTy, Texture>(m_TextureMap, AssetType::Texture, id, outTexture);
//	}
//
//	Result AssetManager::DumpMesh(AssetID id) noexcept
//	{
//		ConstView<Mesh> mesh;
//		Result result = GetAsset<decltype(m_MeshMap), Mesh>(m_MeshMap, AssetType::Mesh, id, mesh);
//
//		if (!result)
//			return result;
//		else if (mesh.Null())
//			return ResultType::Failed;
//		
//		std::string assetIDStr = std::to_string(id.RawHandle());
//
//		std::string filename = "logs/Dump_Mesh_";
//		filename.append(assetIDStr);
//		filename.append(".txt");
//
//		const File* pFile = nullptr;
//		bool isDirect = false;
//		auto loadedFileIt = m_LoadedFiles.find(id);
//		if (loadedFileIt != m_LoadedFiles.end())
//		{
//			pFile = &loadedFileIt->second;
//			isDirect = true;
//		}
//		else
//		{
//			auto parentIt = m_LoadedFiles.find(mesh->ModelID);
//
//			if (parentIt != m_LoadedFiles.end())
//				pFile = &parentIt->second;
//		}
//
//		std::ofstream stream(filename);
//
//		if (!stream)
//		{
//			CM_ENGINE_LOG_WARN(
//				"(AssetManager) Internal warning: Failed to open target "
//				"dump file: {}",
//				filename
//			);
//
//			return ResultType::Failed_File_Serialize;
//		}
//
//		auto now = std::chrono::system_clock::now();
//		time_t timeCurrent = std::chrono::system_clock::to_time_t(now);
//		tm tm = spdlog::details::os::localtime(timeCurrent);
//
//		char timestampBuffer[64];
//		std::strftime(timestampBuffer, sizeof(timestampBuffer), "%Y-%m-%d_%H-%M-%S", &tm);
//
//		/* Evil macro used to avoid extraneous string concatenations. */
//		#define DUMP_SPACING "          "
//		constexpr std::string_view Header = "         [Position]                         [Normal]                         [TexCoord]";
//
//		stream << timestampBuffer << '\n';
//
//		if (pFile != nullptr)
//		{
//			if (isDirect)
//				stream << "Referenced file: ";
//			else
//				stream << "Parent file: ";
//			
//			stream << pFile->Path << "\n\n";
//		}
//
//		stream << Header << '\n';
//		stream << std::fixed << std::setprecision(3);
//		
//		size_t numVertices = mesh->Data.Vertices.size();
//		size_t vIndex = 0;
//		size_t previousDigitsOfIndex = 0;
//		size_t digitsOfNumVertices = std::to_string(numVertices).size();
//
//		std::string uniformSpacing(digitsOfNumVertices, ' ');
//		std::string currentSpacing = uniformSpacing;
//
//		for (const Vertex& v : mesh->Data.Vertices)
//		{
//			std::string vIndexStr = std::to_string(vIndex);
//			size_t currentDigitsOfIndex = vIndexStr.size();
//
//			/* So spacing for each vertex is uniform... */
//			if (vIndexStr.size() != previousDigitsOfIndex)
//				currentSpacing = uniformSpacing.substr(0, uniformSpacing.size() - (currentDigitsOfIndex - 1));
//
//			previousDigitsOfIndex = currentDigitsOfIndex;
//
//			stream << vIndexStr << ':' << currentSpacing;
//			vIndex++;
//
//			DumpFloat3(stream, v.Pos);
//			stream << DUMP_SPACING;
//
//			DumpFloat3(stream, v.Normal);
//			stream << DUMP_SPACING;
//
//			DumpFloat3(stream, v.TexCoord);
//
//			if (vIndex != numVertices)
//				stream << '\n';
//		}
//#undef DUMP_SPACING
//		return ResultType::Succeeded;
//	}
//
//	void AssetManager::DumpFloat3(std::ofstream& stream, const Float3& f3) noexcept
//	{
//		auto clean = [](float v) {
//			return (std::fabs(v) < 1e-6f) ? 0.0f : v;
//		};
//
//		stream << "(";
//
//		if (f3.x >= 0)
//			stream << " ";
//
//		stream << clean(f3.x) << ", ";
//
//		if (f3.y >= 0)
//			stream << " ";
//
//		stream << clean(f3.y) << ", ";
//
//		if (f3.z >= 0)
//			stream << " ";
//
//		stream << clean(f3.z) << ")";
//	}
}