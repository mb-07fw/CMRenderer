#pragma once

#include "Asset/AssetID.hpp"

#include <cstdint>

#include <vector>

namespace Engine::Asset
{
	/*struct Vertex
	{
		Float3 Pos;
		Float3 Normal;
		Float2 TexCoord;
	};

	using Index = uint16_t;

	struct MeshData
	{
		std::vector<Vertex> Vertices;
		std::vector<Index> Indices;
	};

	struct MaterialData
	{
		MaterialData() = default;

		inline MaterialData(
			const Color4& baseColor,
			float metallic,
			float roughness
		) noexcept
			: BaseColor(baseColor),
			Metallic(metallic),
			Roughness(roughness)
		{
		}

		Color4 BaseColor;
		float Metallic = 0.0f;
		float Roughness = 0.0f;
		float Padding[2] = { 0.0f, 0.0f };
	};*/

	struct Asset
	{
		inline Asset(AssetType type) noexcept
			: Type(type)
		{
		}

		virtual ~Asset() = default;

		AssetID ID;
		const AssetType Type = AssetType::Invalid;
	};

	struct Shader : public Asset
	{
		inline Shader() noexcept
			: Asset(AssetType::Shader)
		{
		}

		std::vector<char> RawData; /* Native file; not bytecode. */
	};

	/*struct Model : public Asset
	{
		inline Model() noexcept
			: Asset(AssetType::Model)
		{
		}

		std::vector<AssetID> Meshes;
		std::vector<AssetID> Materials;
	};

	struct Mesh : public Asset
	{
		inline Mesh(const MeshData& data) noexcept
			: Asset(AssetType::Mesh),
			Data(data)
		{
		}

		inline Mesh() noexcept
			: Asset(AssetType::Mesh)
		{
		}

		~Mesh() = default;

		MeshData Data;
		AssetID ModelID;
		uint32_t Index = 0;
	};

	struct Material : public Asset
	{
		inline Material() noexcept
			: Asset(AssetType::Material)
		{
		}

		inline Material(
			AssetID modelID,
			uint32_t index,
			const Color4& baseColor,
			float metallic,
			float roughness
		) noexcept
			: Asset(AssetType::Material),
			  ModelID(modelID),
			  Index(index),
			  Data(baseColor, metallic, roughness)
		{
		}

		Material(const Material&) = default;
		Material(Material&&) = default;
		Material& operator=(const Material&) = default;
		Material& operator=(Material&&) = default;

		MaterialData Data;
		AssetID ModelID;
		uint32_t Index = 0;
	};

	struct Texture : public Asset
	{
		inline Texture() noexcept
			: Asset(AssetType::Texture)
		{
		}

		std::unique_ptr<std::byte> pBuffer;
		size_t SizeBytes = 0;
	};*/
}