#pragma once

#include "CME_Asset.hpp"
#include "CME_MeshSerializer.hpp"

#include <unordered_map>
#include <memory>
#include <format>

namespace CMEngine::Asset
{
	enum class CMAMResultType : int8_t
	{
		INVALID = -127,
		FAILED_FILE_ABSENT,
		FAILED_HANDLE_NOT_REGISTERED,
		FAILED_HANDLE_NOT_MAPPED,
		FAILED_HANDLE_ALREADY_REGISTERED,
		FAILED_HANDLE_ALREADY_MAPPED,
		FAILED_HANDLE_MISMATCHING_ASSET_TYPE,
		FAILED_MISMATCHING_MAPPED_TYPE,
		FAILED_SERIALIZE,
		FAILED = 0,
		SUCCEEDED
	};

	struct CMAMResult
	{
		inline constexpr CMAMResult(CMAMResultType type) noexcept
			: Type(type)
		{
		}

		constexpr ~CMAMResult() = default;

		inline static constexpr [[nodiscard]] int8_t TypeToInt8(CMAMResultType type) noexcept
		{ 
			return static_cast<int8_t>(type); 
		}

		inline static constexpr [[nodiscard]] int16_t TypeToInt16(CMAMResultType type) noexcept
		{
			return static_cast<int16_t>(type);
		}

		inline constexpr operator CMAMResultType() const noexcept { return Type; }

		inline constexpr [[nodiscard]] int8_t ToInt8() const noexcept { return TypeToInt8(Type); }
		inline constexpr [[nodiscard]] int16_t ToInt16() const noexcept { return TypeToInt16(Type); }
		inline constexpr [[nodiscard]] bool Succeeded() const noexcept { return ToInt8() > TypeToInt8(CMAMResultType::FAILED); }
		inline constexpr [[nodiscard]] bool Failed() const noexcept { return ToInt8() < TypeToInt8(CMAMResultType::SUCCEEDED); }
		inline constexpr [[nodiscard]] bool Invalid() const noexcept { return Type == CMAMResultType::INVALID; }

		inline constexpr operator bool() const noexcept { return Succeeded(); }
		inline constexpr [[nodiscard]] bool operator!() const noexcept { return Failed(); }

		CMAMResultType Type = CMAMResultType::INVALID;
	};

	template <typename Ty>
	struct CMAMResultPair
	{
		inline constexpr CMAMResultPair(CMAMResultType type, Ty other) noexcept
			: Result(type),
			  Other(other)
		{
		}

		CMAMResult Result;
		Ty Other;
	};

	using CMAMResultHandle = CMAMResultPair<CMAssetHandle>;
	
	class CMAssetManager
	{
		using GlobalID = uint32_t;
	public:
		CMAssetManager(CMCommon::CMLoggerWide& logger) noexcept;
		~CMAssetManager() noexcept;
	public:
		void Init() noexcept;
		void Shutdown() noexcept;

		/* @meshName should be the file name WITHOUT the file's extension.
		 * Mesh is loaded relatively from CMEngine/resources/assets/meshes/ 
		 * May return an invalid mesh handle if a file with @meshName doesn't exist.
		 */
		[[nodiscard]] CMAMResultHandle LoadMesh(std::string_view meshName) noexcept;

		/* @meshName should be the file name WITHOUT the file's extension. */
		[[nodiscard]] CMAMResult SaveMesh(CMAssetHandle handle, std::string_view meshName) noexcept;

		[[nodiscard]] CMAMResultHandle SaveMeshAsNew(const CMMeshData& meshData, std::string_view meshName) noexcept;

		[[nodiscard]] CMAMResultPair<const CMMesh*> GetMesh(CMAssetHandle handle) noexcept;

		/* Returns true if the handle was registered previously; false otherwise. */
		bool Unregister(CMAssetHandle& outHandle) noexcept;

		[[nodiscard]] bool IsMapped(CMAssetHandle handle) noexcept;
	private:
		[[nodiscard]] CMAMResult SerializeMesh(const CMMeshData& meshData, std::string_view meshName) noexcept;

		[[nodiscard]] uint32_t NextGlobalID() noexcept;

		void CleanupHandle(CMAssetHandle& outHandle) noexcept;
	private:
		CMMeshSerializer m_MeshSerializer;
		uint32_t m_TotalAssetCount = 0;
		std::vector<GlobalID> m_FreeGlobalIDs;
		std::unordered_map<GlobalID, CMMesh> m_MeshMap;
		CMCommon::CMLoggerWide& m_Logger;
		bool m_Initialized = false;
		bool m_Shutdown = false;
	};
}