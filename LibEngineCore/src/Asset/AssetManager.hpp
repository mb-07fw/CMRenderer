#pragma once

#include "Asset/Asset.hpp"
#include "Types.hpp"

#include <unordered_map>
#include <memory>
#include <functional>

namespace CMEngine::Asset
{
	enum class ResultType : int8_t
	{
		Invalid = -127,
		Failed_File_Absent,
		Failed_File_Import,
		Failed_File_Serialize,
		Failed_Handle_Not_Registered,
		Failed_Handle_Not_Mapped,
		Failed_Handle_Already_Registered,
		Failed_Handle_Already_Mapped,
		Failed_Handle_Mismatching_Asset_Type,
		Failed_Handle_Mismatching_Mapped_Type,
		Failed = 0,
		Succeeded
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
			case ResultType::Invalid:							    return std::string_view("Invalid");
			case ResultType::Failed_File_Absent:				    return std::string_view("Failed_File_Absent");
			case ResultType::Failed_File_Import:				    return std::string_view("Failed_File_Import");
			case ResultType::Failed_File_Serialize:					return std::string_view("Failed_File_Serialize");
			case ResultType::Failed_Handle_Not_Registered:		    return std::string_view("Failed_Handle_Not_Registered");
			case ResultType::Failed_Handle_Not_Mapped:			    return std::string_view("Failed_Handle_Not_Mapped");
			case ResultType::Failed_Handle_Already_Registered:	    return std::string_view("Failed_Handle_Already_Registered");
			case ResultType::Failed_Handle_Already_Mapped:		    return std::string_view("Failed_Handle_Already_Mapped");
			case ResultType::Failed_Handle_Mismatching_Asset_Type:  return std::string_view("Failed_Handle_Mismatching_Asset_Type");
			case ResultType::Failed_Handle_Mismatching_Mapped_Type: return std::string_view("Failed_Handle_Mismatching_Mapped_Type");
			case ResultType::Failed:							    return std::string_view("Failed");
			case ResultType::Succeeded:							    return std::string_view("Succeeded");
			default:											    return std::string_view("Unknown");
			}
		}

		inline constexpr operator ResultType() const noexcept { return Type; }

		inline constexpr [[nodiscard]] int8_t ToInt8() const noexcept { return TypeToInt8(Type); }
		inline constexpr [[nodiscard]] int16_t ToInt16() const noexcept { return TypeToInt16(Type); }
		inline constexpr [[nodiscard]] std::string_view ToStringView() const noexcept { return TypeToStringView(Type); }
		inline constexpr [[nodiscard]] bool Succeeded() const noexcept { return Type == ResultType::Succeeded; }
		inline constexpr [[nodiscard]] bool Failed() const noexcept { return Type != ResultType::Succeeded; }
		inline constexpr [[nodiscard]] bool Invalid() const noexcept { return Type == ResultType::Invalid; }

		inline constexpr operator bool() const noexcept { return Succeeded(); }
		inline constexpr [[nodiscard]] bool operator!() const noexcept { return Failed(); }

		ResultType Type = ResultType::Invalid;
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

		/* Mesh and material data can then be retrieved using GetModel(modelID),
		 *    and then GetModel or GetMaterial with any of it's children.
		 * May return an invalid AssetID if a file of @modelPath doesn't exist. */
		Result LoadModel(const std::filesystem::path& modelPath, AssetID& outModelID) noexcept;
		
		/* Note: The underlying pointers may become invalidated if the corresponding bucket of the asset type resizes via asset loading.
		 *       Best practice should be ensuring ConstView's are temporary and synchronous. */
		Result GetModel(AssetID id, ConstView<Model>& outModel) noexcept;
		Result GetMesh(AssetID id, ConstView<Mesh>& outMesh) noexcept;
		Result GetMaterial(AssetID id, ConstView<Material>& outMaterial) noexcept;

		/* Returns true if the handle was registered previously; false otherwise. */
		bool Unregister(AssetID& outID) noexcept;

		[[nodiscard]] bool IsMapped(AssetID id) noexcept;
	private:
		template <typename MapTy, typename AssetTy>
		inline [[nodiscard]] Result GetAsset(
			const MapTy& map,
			AssetType assetType,
			AssetID id,
			ConstView<AssetTy>& outAsset
		) noexcept;

		[[nodiscard]] uint32_t NextGlobalID() noexcept;

		void CleanupID(AssetID& outID) noexcept;
	private:
		uint32_t m_TotalAssetCount = 0;
		std::unique_ptr<ModelImporterImpl> mP_ModelImporter;
		std::vector<GlobalID> m_FreeGlobalIDs;
		std::unordered_map<AssetID, Model> m_ModelMap;
		std::unordered_map<AssetID, Mesh> m_MeshMap;
		std::unordered_map<AssetID, Material> m_MaterialMap;
	};

	template <typename MapTy, typename AssetTy>
	inline [[nodiscard]] Result AssetManager::GetAsset(const MapTy& map, AssetType assetType, AssetID id, ConstView<AssetTy>& outAsset) noexcept
	{
		outAsset = nullptr;

		if (id.Type() != assetType)
			return ResultType::Failed_Handle_Mismatching_Asset_Type;
		else if (!id.IsRegistered())
			return ResultType::Failed_Handle_Not_Registered;

		auto it = map.find(id);
		if (it == map.end())
			return ResultType::Failed_Handle_Not_Mapped;

		outAsset = &it->second;
		return ResultType::Succeeded;
	}
}