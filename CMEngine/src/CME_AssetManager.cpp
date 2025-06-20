#include "CME_PCH.hpp"
#include "CME_AssetManager.hpp"
#include "CME_Paths.hpp"
#include "CMC_Macros.hpp"

namespace CMEngine::Asset
{
	CMAssetManager::CMAssetManager(CMCommon::CMLoggerWide& logger) noexcept
		: m_Logger(logger),
		  m_MeshSerializer(logger)
	{
	}

	CMAssetManager::~CMAssetManager() noexcept
	{
	}

	void CMAssetManager::Init() noexcept
	{
		m_Initialized = true;
		m_Shutdown = false;
	}

	void CMAssetManager::Shutdown() noexcept
	{
		m_Initialized = false;
		m_Shutdown = true;
	}

	[[nodiscard]] CMAMResultHandle CMAssetManager::LoadMesh(std::string_view meshName) noexcept
	{
		CMAssetHandle handle = CMAssetHandle::Invalid();

		std::filesystem::path meshPath = CMEAssetMeshDirectory() / meshName;
		meshPath.replace_extension(".yaml");

		if (!std::filesystem::exists(meshPath))
			return CMAMResultPair(CMAMResultType::FAILED_FILE_ABSENT, handle);

		CMMeshData data = {};
		m_MeshSerializer.LoadMesh(meshPath, data);

		handle = CMAssetHandle::Registered(CMAssetType::MESH, NextGlobalID());

		m_MeshMap.try_emplace(handle.GlobalID(), data);

		return CMAMResultPair(CMAMResultType::SUCCEEDED, handle);
	}

	[[nodiscard]] CMAMResult CMAssetManager::SaveMesh(CMAssetHandle handle, std::string_view meshName) noexcept
	{
		if (handle.Type() != CMAssetType::MESH)
			return CMAMResult(CMAMResultType::FAILED_HANDLE_MISMATCHING_ASSET_TYPE);
		else if (!handle.IsRegistered())
			return CMAMResult(CMAMResultType::FAILED_HANDLE_NOT_REGISTERED);

		auto it = m_MeshMap.find(handle.GlobalID());

		if (it == m_MeshMap.end())
			return CMAMResult(CMAMResultType::FAILED_HANDLE_NOT_MAPPED);

		return SerializeMesh(it->second.Data, meshName);
	}

	[[nodiscard]] CMAMResultHandle CMAssetManager::SaveMeshAsNew(const CMMeshData& meshData, std::string_view meshName) noexcept
	{
		CMAssetHandle handle = CMAssetHandle::Invalid();

		CMAMResult result = SerializeMesh(meshData, meshName);

		if (!result)
			return CMAMResultPair<CMAssetHandle>(result, handle);

		handle = CMAssetHandle::Registered(CMAssetType::MESH, NextGlobalID());

		m_MeshMap[handle.GlobalID()] = CMMesh(meshData);
		return CMAMResultPair<CMAssetHandle>(result, handle);
	}

	[[nodiscard]] CMAMResultPair<const CMMesh*> CMAssetManager::GetMesh(CMAssetHandle handle) noexcept
	{
		using ReturnTy = CMAMResultPair<const CMMesh*>;

		if (handle.Type() != CMAssetType::MESH)
			return ReturnTy(
				CMAMResultType::FAILED_HANDLE_MISMATCHING_ASSET_TYPE,
				nullptr
			);
		else if (!handle.IsRegistered())
			return ReturnTy(
				CMAMResultType::FAILED_HANDLE_NOT_REGISTERED,
				nullptr
			);

		auto it = m_MeshMap.find(handle.GlobalID());

		if (it == m_MeshMap.end())
			return ReturnTy(
				CMAMResultType::FAILED_HANDLE_NOT_MAPPED,
				nullptr
			);

		return ReturnTy(CMAMResultType::SUCCEEDED, &it->second);
	}

	bool CMAssetManager::Unregister(CMAssetHandle& outHandle) noexcept
	{
		if (!outHandle.IsRegistered())
			return false;

		CleanupHandle(outHandle);
		return true;
	}

	[[nodiscard]] bool CMAssetManager::IsMapped(CMAssetHandle handle) noexcept
	{
		switch (handle.Type())
		{
		case CMAssetType::MESH:
			return m_MeshMap.contains(handle.GlobalID());
		case CMAssetType::INVALID: [[fallthrough]];
		case CMAssetType::TEXTURE: [[fallthrough]];
		default:
			return false;
		}
	}

	[[nodiscard]] CMAMResult CMAssetManager::SerializeMesh(const CMMeshData& meshData, std::string_view meshName) noexcept
	{
		std::filesystem::path meshPath = CMEAssetMeshDirectory() / meshName;
		meshPath.replace_extension(".yaml");

		if (!m_MeshSerializer.SaveMesh(meshPath, meshData))
			return CMAMResult(CMAMResultType::FAILED_SERIALIZE);

		return CMAMResult(CMAMResultType::SUCCEEDED);
	}

	[[nodiscard]] uint32_t CMAssetManager::NextGlobalID() noexcept
	{
		if (m_FreeGlobalIDs.empty())
			return m_TotalAssetCount++;

		uint32_t next = m_FreeGlobalIDs.back();
		m_FreeGlobalIDs.pop_back();

		return next;
	}

	void CMAssetManager::CleanupHandle(CMAssetHandle& outHandle) noexcept
	{
		if (!outHandle.IsRegistered() || outHandle.Type() == CMAssetType::INVALID)
			return;

		outHandle.SetRegistered(false);

		switch (outHandle.Type())
		{
		case CMAssetType::MESH:
			if (auto it = m_MeshMap.find(outHandle.GlobalID()); it != m_MeshMap.end())
				m_MeshMap.erase(it);

			break;
		case CMAssetType::TEXTURE:
			break;
		default:
			return;
		}

		m_FreeGlobalIDs.emplace_back(outHandle.GlobalID());
	}
}