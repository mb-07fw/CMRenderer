#include "Core/PCH.hpp"
#include "Core/AssetManager.hpp"
#include "Core/Paths.hpp"
#include "Common/Macros.hpp"

namespace CMEngine::Core::Asset
{
	AssetManager::AssetManager(Common::LoggerWide& logger) noexcept
		: m_Logger(logger),
		  m_MeshSerializer(logger)
	{
	}

	AssetManager::~AssetManager() noexcept
	{
	}

	void AssetManager::Init() noexcept
	{
		m_Initialized = true;
		m_Shutdown = false;
	}

	void AssetManager::Shutdown() noexcept
	{
		m_Initialized = false;
		m_Shutdown = true;
	}

	[[nodiscard]] AMResultHandle AssetManager::LoadMesh(std::string_view meshName) noexcept
	{
		AssetHandle handle = AssetHandle::Invalid();

		std::filesystem::path meshPath = CMEAssetMeshDirectory() / meshName;
		meshPath.replace_extension(".yaml");

		if (!std::filesystem::exists(meshPath))
			return AMResultPair(AMResultType::FAILED_FILE_ABSENT, handle);

		MeshData data = {};
		m_MeshSerializer.LoadMesh(meshPath, data);

		handle = AssetHandle::Registered(AssetType::MESH, NextGlobalID());

		m_MeshMap.try_emplace(handle.GlobalID(), data);

		return AMResultPair(AMResultType::SUCCEEDED, handle);
	}

	[[nodiscard]] AMResult AssetManager::SaveMesh(AssetHandle handle, std::string_view meshName) noexcept
	{
		if (handle.Type() != AssetType::MESH)
			return AMResult(AMResultType::FAILED_HANDLE_MISMATCHING_ASSET_TYPE);
		else if (!handle.IsRegistered())
			return AMResult(AMResultType::FAILED_HANDLE_NOT_REGISTERED);

		auto it = m_MeshMap.find(handle.GlobalID());

		if (it == m_MeshMap.end())
			return AMResult(AMResultType::FAILED_HANDLE_NOT_MAPPED);

		return SerializeMesh(it->second.Data, meshName);
	}

	[[nodiscard]] AMResultHandle AssetManager::SaveMeshAsNew(const MeshData& meshData, std::string_view meshName) noexcept
	{
		AssetHandle handle = AssetHandle::Invalid();

		AMResult result = SerializeMesh(meshData, meshName);

		if (!result)
			return AMResultPair<AssetHandle>(result, handle);

		handle = AssetHandle::Registered(AssetType::MESH, NextGlobalID());

		m_MeshMap[handle.GlobalID()] = Mesh(meshData);
		return AMResultPair<AssetHandle>(result, handle);
	}

	[[nodiscard]] AMResultPair<const Mesh*> AssetManager::GetMesh(AssetHandle handle) noexcept
	{
		using ReturnTy = AMResultPair<const Mesh*>;

		if (handle.Type() != AssetType::MESH)
			return ReturnTy(
				AMResultType::FAILED_HANDLE_MISMATCHING_ASSET_TYPE,
				nullptr
			);
		else if (!handle.IsRegistered())
			return ReturnTy(
				AMResultType::FAILED_HANDLE_NOT_REGISTERED,
				nullptr
			);

		auto it = m_MeshMap.find(handle.GlobalID());

		if (it == m_MeshMap.end())
			return ReturnTy(
				AMResultType::FAILED_HANDLE_NOT_MAPPED,
				nullptr
			);

		return ReturnTy(AMResultType::SUCCEEDED, &it->second);
	}

	bool AssetManager::Unregister(AssetHandle& outHandle) noexcept
	{
		if (!outHandle.IsRegistered())
			return false;

		CleanupHandle(outHandle);
		return true;
	}

	[[nodiscard]] bool AssetManager::IsMapped(AssetHandle handle) noexcept
	{
		switch (handle.Type())
		{
		case AssetType::MESH:
			return m_MeshMap.contains(handle.GlobalID());
		case AssetType::INVALID: [[fallthrough]];
		case AssetType::TEXTURE: [[fallthrough]];
		default:
			return false;
		}
	}

	[[nodiscard]] AMResult AssetManager::SerializeMesh(const MeshData& meshData, std::string_view meshName) noexcept
	{
		std::filesystem::path meshPath = CMEAssetMeshDirectory() / meshName;
		meshPath.replace_extension(".yaml");

		if (!m_MeshSerializer.SaveMesh(meshPath, meshData))
			return AMResult(AMResultType::FAILED_SERIALIZE);

		return AMResult(AMResultType::SUCCEEDED);
	}

	[[nodiscard]] uint32_t AssetManager::NextGlobalID() noexcept
	{
		if (m_FreeGlobalIDs.empty())
			return m_TotalAssetCount++;

		uint32_t next = m_FreeGlobalIDs.back();
		m_FreeGlobalIDs.pop_back();

		return next;
	}

	void AssetManager::CleanupHandle(AssetHandle& outHandle) noexcept
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