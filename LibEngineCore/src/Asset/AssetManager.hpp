#pragma once

#include "Asset/Asset.hpp"
#include "Types.hpp"

#include <unordered_map>
#include <memory>

namespace CMEngine::Asset
{
	enum class ResultType : int8_t
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

	struct Result
	{
		inline constexpr Result(ResultType type) noexcept
			: Type(type)
		{
		}

		constexpr ~Result() = default;

		inline static constexpr [[nodiscard]] int8_t TypeToInt8(ResultType type) noexcept
		{
			return static_cast<int8_t>(type);
		}

		inline static constexpr [[nodiscard]] int16_t TypeToInt16(ResultType type) noexcept
		{
			return static_cast<int16_t>(type);
		}

		inline static constexpr [[nodiscard]] std::string_view TypeToStringView(ResultType type) noexcept
		{
			switch (type)
			{
			case ResultType::INVALID:							   return std::string_view("INVALID");
			case ResultType::FAILED_FILE_ABSENT:				   return std::string_view("FAILED_FILE_ABSENT");
			case ResultType::FAILED_HANDLE_NOT_REGISTERED:		   return std::string_view("FAILED_HANDLE_NOT_REGISTERED");
			case ResultType::FAILED_HANDLE_NOT_MAPPED:			   return std::string_view("FAILED_HANDLE_NOT_MAPPED");
			case ResultType::FAILED_HANDLE_ALREADY_REGISTERED:	   return std::string_view("FAILED_HANDLE_ALREADY_REGISTERED");
			case ResultType::FAILED_HANDLE_ALREADY_MAPPED:		   return std::string_view("FAILED_HANDLE_ALREADY_MAPPED");
			case ResultType::FAILED_HANDLE_MISMATCHING_ASSET_TYPE: return std::string_view("FAILED_HANDLE_MISMATCHING_ASSET_TYPE");
			case ResultType::FAILED_MISMATCHING_MAPPED_TYPE:	   return std::string_view("FAILED_MISMATCHING_MAPPED_TYPE");
			case ResultType::FAILED_SERIALIZE:					   return std::string_view("FAILED_SERIALIZE");
			case ResultType::FAILED:							   return std::string_view("FAILED");
			case ResultType::SUCCEEDED:							   return std::string_view("SUCCEEDED");
			default:											   return std::string_view("UNKNOWN");
			}
		}

		inline constexpr operator ResultType() const noexcept { return Type; }

		inline constexpr [[nodiscard]] int8_t ToInt8() const noexcept { return TypeToInt8(Type); }
		inline constexpr [[nodiscard]] int16_t ToInt16() const noexcept { return TypeToInt16(Type); }
		inline constexpr [[nodiscard]] std::string_view ToStringView() const noexcept { return TypeToStringView(Type); }
		inline constexpr [[nodiscard]] bool Succeeded() const noexcept { return ToInt8() > TypeToInt8(ResultType::FAILED); }
		inline constexpr [[nodiscard]] bool Failed() const noexcept { return ToInt8() < TypeToInt8(ResultType::SUCCEEDED); }
		inline constexpr [[nodiscard]] bool Invalid() const noexcept { return Type == ResultType::INVALID; }

		inline constexpr operator bool() const noexcept { return Succeeded(); }
		inline constexpr [[nodiscard]] bool operator!() const noexcept { return Failed(); }

		ResultType Type = ResultType::INVALID;
	};

	using GlobalID = uint32_t;

	class ModelImporterImpl;

	class AssetManager
	{
	public:
		AssetManager() noexcept;
		~AssetManager() noexcept;
	public:
		void Init() noexcept;
		void Shutdown() noexcept;

		/* @modelName should be the file name WITHOUT the file's extension.
		 * Mesh data can then be retrieved using AssetManager::LoadMesh.
		 * May return an invalid mesh handle if a file with @modelName doesn't exist. */
		[[nodiscard]] Result LoadMesh(const std::filesystem::path& modelPath, AssetID& outID) noexcept;
		
		[[nodiscard]] Result GetMesh(AssetID id, ConstView<Mesh>& pOutMesh) noexcept;

		/* Returns true if the handle was registered previously; false otherwise. */
		bool Unregister(AssetID& outID) noexcept;

		[[nodiscard]] bool IsMapped(AssetID id) noexcept;
	private:
		[[nodiscard]] uint32_t NextGlobalID() noexcept;

		void CleanupID(AssetID& outID) noexcept;
	private:
		uint32_t m_TotalAssetCount = 0;
		std::unique_ptr<ModelImporterImpl> mP_ModelImporter;
		std::vector<GlobalID> m_FreeGlobalIDs;
		std::unordered_map<GlobalID, Mesh> m_MeshMap;
	};
}