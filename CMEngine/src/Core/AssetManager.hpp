#pragma once

#include <unordered_map>
#include <memory>
#include <format>

#include "Core/Asset.hpp"
#include "Core/MeshSerializer.hpp"

namespace CMEngine::Core::Asset
{
	enum class AMResultType : int8_t
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

	struct AMResult
	{
		inline constexpr AMResult(AMResultType type) noexcept
			: Type(type)
		{
		}

		constexpr ~AMResult() = default;

		inline static constexpr [[nodiscard]] int8_t TypeToInt8(AMResultType type) noexcept
		{ 
			return static_cast<int8_t>(type); 
		}

		inline static constexpr [[nodiscard]] int16_t TypeToInt16(AMResultType type) noexcept
		{
			return static_cast<int16_t>(type);
		}

		inline constexpr operator AMResultType() const noexcept { return Type; }

		inline constexpr [[nodiscard]] int8_t ToInt8() const noexcept { return TypeToInt8(Type); }
		inline constexpr [[nodiscard]] int16_t ToInt16() const noexcept { return TypeToInt16(Type); }
		inline constexpr [[nodiscard]] bool Succeeded() const noexcept { return ToInt8() > TypeToInt8(AMResultType::FAILED); }
		inline constexpr [[nodiscard]] bool Failed() const noexcept { return ToInt8() < TypeToInt8(AMResultType::SUCCEEDED); }
		inline constexpr [[nodiscard]] bool Invalid() const noexcept { return Type == AMResultType::INVALID; }

		inline constexpr operator bool() const noexcept { return Succeeded(); }
		inline constexpr [[nodiscard]] bool operator!() const noexcept { return Failed(); }

		AMResultType Type = AMResultType::INVALID;
	};

	template <typename Ty>
	struct AMResultPair
	{
		inline constexpr AMResultPair(AMResultType type, Ty other) noexcept
			: Result(type),
			  Other(other)
		{
		}

		AMResult Result;
		Ty Other;
	};

	using AMResultHandle = AMResultPair<AssetHandle>;
	
	class AssetManager
	{
		using GlobalID = uint32_t;
	public:
		AssetManager(Common::LoggerWide& logger) noexcept;
		~AssetManager() noexcept;
	public:
		void Init() noexcept;
		void Shutdown() noexcept;

		/* @meshName should be the file name WITHOUT the file's extension.
		 * Mesh is loaded relatively from CMEngine/resources/assets/meshes/ 
		 * May return an invalid mesh handle if a file with @meshName doesn't exist.
		 */
		[[nodiscard]] AMResultHandle LoadMesh(std::string_view meshName) noexcept;

		/* @meshName should be the file name WITHOUT the file's extension. */
		[[nodiscard]] AMResult SaveMesh(AssetHandle handle, std::string_view meshName) noexcept;

		[[nodiscard]] AMResultHandle SaveMeshAsNew(const MeshData& meshData, std::string_view meshName) noexcept;

		[[nodiscard]] AMResultPair<const Mesh*> GetMesh(AssetHandle handle) noexcept;

		/* Returns true if the handle was registered previously; false otherwise. */
		bool Unregister(AssetHandle& outHandle) noexcept;

		[[nodiscard]] bool IsMapped(AssetHandle handle) noexcept;
	private:
		[[nodiscard]] AMResult SerializeMesh(const MeshData& meshData, std::string_view meshName) noexcept;

		[[nodiscard]] uint32_t NextGlobalID() noexcept;

		void CleanupHandle(AssetHandle& outHandle) noexcept;
	private:
		MeshSerializer m_MeshSerializer;
		uint32_t m_TotalAssetCount = 0;
		std::vector<GlobalID> m_FreeGlobalIDs;
		std::unordered_map<GlobalID, Mesh> m_MeshMap;
		Common::LoggerWide& m_Logger;
		bool m_Initialized = false;
		bool m_Shutdown = false;
	};
}