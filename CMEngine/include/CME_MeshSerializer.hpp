#pragma once

#include <string>
#include <string_view>
#include <filesystem>
#include <type_traits>
#include <array>

#include "CME_MeshData.hpp"
#include "CMC_Logger.hpp"
#include "CMC_BinaryStream.hpp"

#include "yaml-cpp/yaml.h"

namespace CMEngine::Asset
{
#pragma region Parsing Errors
	enum class CMMeshParsingErrorType : int8_t
	{
		INVALID = -1,
		FILE_READ_FAILED_OPEN,
		FILE_ROOT_READ_FAILED_RETRIEVAL,
		MESH_ROOT_READ_FAILED_RETRIEVAL,
		DESCRIPTOR_READ_FAILED_RETRIEVAL,
		DESCRIPTOR_MESH_NAME_READ_FAILED_RETRIEVAL,
		DESCRIPTOR_VERTEX_COUNT_READ_FAILED_RETRIEVAL,
		DESCRIPTOR_INDEX_COUNT_READ_FAILED_RETRIEVAL,
		DESCRIPTOR_VERTEX_BYTE_STRIDE_AND_HINT_READ_FAILED_RETRIEVAL,
		DESCRIPTOR_INDEX_BYTE_STRIDE_AND_HINT_READ_FAILED_RETRIEVAL,
		DESCRIPTOR_ATTRIBUTES_READ_FAILED_RETRIEVAL,
		DESCRIPTOR_ATTRIBUTES_NAME_READ_FAILED_RETRIEVAL,
		DESCRIPTOR_ATTRIBUTES_BYTE_OFFSET_READ_FAILED_RETRIEVAL,
		DESCRIPTOR_VERTEX_COUNT_PARSE_ZERO,
		DESCRIPTOR_VERTEX_COUNT_PARSE_FAILED,
		DESCRIPTOR_INDEX_COUNT_PARSE_ZERO,
		DESCRIPTOR_INDEX_COUNT_PARSE_FAILED,
		DESCRIPTOR_VERTEX_BYTE_STRIDE_PARSE_ZERO,
		DESCRIPTOR_VERTEX_BYTE_STRIDE_AND_HINT_PARSE_FAILED,
		DESCRIPTOR_VERTEX_BYTE_STRIDE_HINT_PARSE_FAILED,
		DESCRIPTOR_VERTEX_BYTE_STRIDE_HINT_PARSE_INVALID,
		DESCRIPTOR_INDEX_BYTE_STRIDE_PARSE_ZERO,
		DESCRIPTOR_INDEX_BYTE_STRIDE_AND_HINT_PARSE_FAILED,
		DESCRIPTOR_INDEX_BYTE_STRIDE_HINT_PARSE_FAILED,
		DESCRIPTOR_INDEX_BYTE_STRIDE_HINT_PARSE_INVALID,
		DESCRIPTOR_ATTRIBUTES_PARSE_ABSENT,
		DESCRIPTOR_ATTRIBUTES_PARSE_NON_SEQUENCE,
		DESCRIPTOR_ATTRIBUTES_TYPE_HINT_PARSE_FAILED,
		DESCRIPTOR_ATTRIBUTES_TYPE_HINT_PARSE_INVALID,
		DESCRIPTOR_ATTRIBUTES_BYTE_OFFSET_PARSE_FAILED,
		DESCRIPTOR_ATTRIBUTES_BYTE_OFFSET_PARSE_INVALID_FIRST,
		DESCRIPTOR_BINARY_BEGIN_BYTE_POS_INVALID,
		DATA_READ_FAILED_RETRIEVAL,
		DATA_VERTICES_READ_FAILED_RETRIEVAL,
		DATA_INDICES_READ_FAILED_RETRIEVAL,
		DATA_VERTICES_PARSE_FAILED,
		DATA_INDICES_PARSE_FAILED,
		TOTAL /* Technically there are TOTAL + 1 elements, but INVALID is just a placeholder. */
	};
	
	/* Technically there are CMMeshParingErrorType::TOTAL + 1 elements, but only CMMeshParsingErrorType::TOTAL messages. */
	inline constexpr std::array<std::wstring_view, static_cast<size_t>(CMMeshParsingErrorType::TOTAL)> G_MESH_PARSING_ERROR_MESSAGES = { {
		/* CMMeshParsingErrorType::FILE_READ_FAILED_OPEN */
		{ L"Failed to open .yaml file : `{}`. Does it exist?" },

		/* CMMeshParsingErrorType::FILE_ROOT_READ_FAILED_RETRIEVAL */
		{ L"Failed to retrieve root yaml node of mesh file : `{}`." },

		/* CMMeshParsingErrorType::MESH_ROOT_READ_FAILED_RETRIEVAL */
		{ L"Failed to retrieve CMSerializedMesh node of mesh file `{}`." },

		/* CMMeshParsingErrorType::DESCRIPTOR_READ_FAILED_RETRIEVAL */
		{ L"Failed to retrieve Descriptor node of mesh descriptor : `{}`." },

		/* CMMeshParsingErrorType::DESCRIPTOR_MESH_NAME_READ_FAILED_RETRIEVAL */
		{ L"Failed to retrieve MeshName node of mesh descriptor : `{}`." },

		/* CMMeshParsingErrorType::DESCRIPTOR_VERTEX_COUNT_READ_FAILED_RETRIEVAL */
		{ L"Failed to retrieve VertexCount node of mesh descriptor : `{}`." },

		/* CMMeshParsingErrorType::DESCRIPTOR_INDEX_COUNT_READ_FAILED_RETRIEVAL */
		{ L"Failed to retrieve IndexCount node of mesh descriptor : `{}`." },

		/* CMMeshParsingErrorType::DESCRIPTOR_VERTEX_BYTE_STRIDE_AND_HINT_READ_FAILED_RETRIEVAL */
		{ L"Failed to retrieve valid vertex byte stride from VertexByteStride "
		  L"or VertexByteStrideHint nodes from mesh descriptor : `{}`."
		},

		/* CMMeshParsingErrorType::DESCRIPTOR_INDEX_BYTE_STRIDE_AND_HINT_READ_FAILED_RETRIEVAL */
		{ L"Failed to retrieve valid index byte stride from IndexByteStride "
		  L"or IndexByteStrideHint nodes from mesh descriptor : `{}`."
		},

		/* CMMeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_READ_FAILED_RETRIEVAL */
		{ L"Failed to retrieve Attributes node from mesh descriptor : `{}`." },

		/* CMMeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_NAME_READ_FAILED_RETRIEVAL */
		{ L"Failed to retrieve Name node from attribute `{}`. Mesh descriptor : `{}`." },

		/* CMMeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_BYTE_OFFSET_READ_FAILED_RETRIEVAL */
		{ L"Failed to retrieve ByteOffset node from attribute `{}`. Mesh descriptor : `{}`." },

		/* CMMeshParsingErrorType::DESCRIPTOR_VERTEX_COUNT_PARSE_ZERO */
		{ L"Vertex count is 0. A serialized mesh should have vertices to serialize. Mesh descriptor : `{}`." },

		/* CMMeshParsingErrorType::DESCRIPTOR_VERTEX_COUNT_PARSE_FAILED */
		{ L"Failed to parse VertexCount node from mesh descriptor : `{}`." },

		/* CMMeshParsingErrorType::DESCRIPTOR_INDEX_COUNT_PARSE_ZERO */
		{ L"Index count is 0. A serialized mesh should have indices to serialize. Mesh descriptor : `{}`." },

		/* CMMeshParsingErrorType::DESCRIPTOR_INDEX_COUNT_PARSE_FAILED */
		{ L"Failed to parse IndexCount node from mesh descriptor : `{}`." },

		/* CMMeshParsingErrorType::DESCRIPTOR_VERTEX_BYTE_STRIDE_PARSE_ZERO */
		{ L"Vertex byte stride is 0. It is imperitive that a serialized mesh has "
		  L"a valid vertex byte stride defined in order to successfully parse it's binary data. "
		  L"Mesh descriptor : `{}`."
		},

		/* CMMeshParsingErrorType::DESCRIPTOR_VERTEX_BYTE_STRIDE_AND_HINT_PARSE_FAILED */
		{ L"Failed to parse VertexByteStride and no VertexByteStrideHint is "
		  L"defined, or VertexByteStrideHint is CMTypeHint::NONE (`none`). "
		  L"Mesh descriptor : `{}`."
		},

		/* CMMeshParsingErrorType::DESCRIPTOR_VERTEX_BYTE_STRIDE_HINT_PARSE_FAILED */
		{ L"Failed to parse VertexByteStrideHint node from mesh descriptor : `{}`. "
		  L"If VertexByteStride was intended to be prioritzed, VertexByteStrideHint "
		  L"should be CMTypeHint::NONE (`none` or left undefined in parsing)."
		},

		/* CMMeshParsingErrorType::DESCRIPTOR_VERTEX_BYTE_STRIDE_HINT_PARSE_INVALID */
		{ L"Vertex byte stride hint is CMTypeHint::INVALID (`invalid`). This implies that the parsed type "
		  L"hint had the value CMTypeHint::INVALID, which is illegal, instead of the abscence "
		  L"of a CMTypeHint being described by CMTypeHint::NONE. If VertexByteStride was intended "
		  L"to be prioritized, VertexByteStrideHint should be CMTypeHint::NONE (`none` or left undefined in parsing). "
		  L"Mesh descriptor : `{}`."
		},

		/* CMMeshParsingErrorType::DESCRIPTOR_INDEX_BYTE_STRIDE_PARSE_ZERO */
		{ L"Index byte stride is 0. It is imperitive that a serialized mesh has an index byte "
		  L"stride defined in order to successfully parse it's binary data. Mesh descriptor : `{}`."
		},

		/* CMMeshParsingErrorType::DESCRIPTOR_INDEX_BYTE_STRIDE_AND_HINT_PARSE_FAILED */
		{ L"Failed to parse IndexByteStride and no IndexByteStrideHint is defined, or IndexByteStrideHint is "
		  L"CMHintType::NONE. Mesh descriptor : `{}`."
	    },

		/* CMMeshParsingErrorType::DESCRIPTOR_INDEX_BYTE_STRIDE_HINT_PARSE_FAILED */
		{ L"Failed to parse IndexByteStrideHint node from mesh descriptor : `{}`. "
		  L"If VertexByteStride was intended to be prioritzed, VertexByteStrideHint "
		  L"should be CMTypeHint::NONE (`none` or left undefined in parsing)."
		},

		/* CMMeshParsingErrorType::DESCRIPTOR_INDEX_BYTE_STRIDE_HINT_PARSE_INVALID */
		{ L"Index byte stride hint is CMTypeHint::INVALID. This implies that the parsed type hint had the value "
		  L"CMTypeHint::INVALID, which is illegal, instead of the abscence of a CMTypeHint being described by "
		  L"CMTypeHint::NONE. If IndexByteStride was intended to be prioritized, IndexByteStrideHint should be "
		  L"CMTypeHint::NONE (`none` or left undefined in parsing). Mesh descriptor : `{}`."
		},
		
		/* CMMeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_PARSE_ABSENT */
		{ L"No vertex attributes are present. It is imperitive that a serialized mesh has vertex attributes "
		  L"defined in order to successfully parsed it's binary data. Mesh descriptor : `{}`."
		},

		/* CMMeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_PARSE_NON_SEQUENCE */
		{ L"VertexAttributes node isn't a sequence. This means the node doesn't contain a list of "
		  L"attributes which is illegal for a serialized mesh."
		},

		/* CMMeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_TYPE_HINT_PARSE_FAILED */
		{ L"Failed to parse a valid CMTypeHint from TypeHint node. If TypeHint of the VertexAttribute " 
		  L"was intended to be discarded due to the data type not being compatible, TypeHint should "
		  L"have the string value `none` or shouldn't be defined. Attribute `{}`, Mesh descriptor : `{}`."
		},

		/* CMMeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_TYPE_HINT_PARSE_INVALID */
		{ L"TypeHint node has the value CMTypeHint::INVALID. "
		  L"If TypeHint of the VertexAttribute was intended to be discarded due to the data type "
		  L"not being compatible with a CMTypeHint, TypeHint should have the string value `none` or shouldn't be defined. "
		  L"Attribute `{}`, Mesh descriptor : `{}`."
		},

		/* CMMeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_BYTE_OFFSET_PARSE_FAILED */
		{ L"Failed to parse a valid byte offset from ByteOffset node from attribute. `{}`. Mesh descriptor : `{}`." },

		/* CMMeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_BYTE_OFFSET_PARSE_INVALID_FIRST */
		{ L"The first vertex attribute should have a byte offset of 0. A byte offset of `{}` for the first attribute "
		  L"implies you want to skip `{}` bytes of the beginning of each vertex, which is most likely malformed. "
		  L"Mesh descriptor : `{}`."
		},

		/* CMMeshParsingErrorType::DESCRIPTOR_BINARY_BEGIN_BYTE_POS_INVALID */
		{ L"The mesh descriptor doesn't have a valid byte offset of which to read in it's "
		  L"binary data. Byte offset : `{}`, Mesh descriptor : `{}`."
		},

		/* CMMeshParsingErrorType::DATA_READ_FAILED_RETRIEVAL */
		{ L"Failed to retrieve Data node from mesh file : `{}`." },

		/* CMMeshParsingErrorType::DATA_VERTICES_READ_FAILED_RETRIEVAL */
		{ L"Failed to retrieve Vertices node from mesh file : `{}`." },

		/* CMMeshParsingErrorType::DATA_INDICES_READ_FAILED_RETRIEVAL */
		{ L"Failed to retrieve Indices node from mesh file : `{}`." },

		/* CMMeshParsingErrorType::DATA_VERTICES_PARSE_FAILED */
		{ L"Failed to parse binary from Vertices node from mesh file : `{}`." },

		/* CMMeshParsingErrorType::DATA_INDICES_PARSE_FAILED */
		{ L"Failed to parse binary from Indices node from mesh file : `{}`." },
	} };

	inline constexpr [[nodiscard]] int8_t MeshParsingErrorIndex(CMMeshParsingErrorType errorType) noexcept
	{
		static_assert(static_cast<size_t>(CMMeshParsingErrorType::TOTAL) == G_MESH_PARSING_ERROR_MESSAGES.size(),
			"Mismatch between error enum and message array.");

		return static_cast<int8_t>(errorType);
	}

	inline constexpr [[nodiscard]] std::wstring_view MeshParsingErrorMessage(CMMeshParsingErrorType errorType) noexcept
	{
		static_assert(static_cast<size_t>(CMMeshParsingErrorType::TOTAL) == G_MESH_PARSING_ERROR_MESSAGES.size(),
			"Mismatch between error enum and message array.");

		int8_t index = MeshParsingErrorIndex(errorType);

		if (index < 0 || index >= G_MESH_PARSING_ERROR_MESSAGES.size())
			return std::wstring_view(L"");

		return G_MESH_PARSING_ERROR_MESSAGES[index];
	}

	inline constexpr std::wstring_view G_ERMW_FILE_READ_FAILED_OPEN =
		MeshParsingErrorMessage(CMMeshParsingErrorType::FILE_READ_FAILED_OPEN);

	inline constexpr std::wstring_view G_ERMW_FILE_ROOT_READ_FAILED_RETRIEVAL =
		MeshParsingErrorMessage(CMMeshParsingErrorType::FILE_ROOT_READ_FAILED_RETRIEVAL);

	inline constexpr std::wstring_view G_ERMW_MESH_ROOT_READ_FAILED_RETRIEVAL =
		MeshParsingErrorMessage(CMMeshParsingErrorType::MESH_ROOT_READ_FAILED_RETRIEVAL);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_READ_FAILED_RETRIEVAL =
		MeshParsingErrorMessage(CMMeshParsingErrorType::DESCRIPTOR_READ_FAILED_RETRIEVAL);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_MESH_NAME_READ_FAILED_RETRIEVAL =
		MeshParsingErrorMessage(CMMeshParsingErrorType::DESCRIPTOR_MESH_NAME_READ_FAILED_RETRIEVAL);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_VERTEX_COUNT_READ_FAILED_RETRIEVAL =
		MeshParsingErrorMessage(CMMeshParsingErrorType::DESCRIPTOR_VERTEX_COUNT_READ_FAILED_RETRIEVAL);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_INDEX_COUNT_READ_FAILED_RETRIEVAL =
		MeshParsingErrorMessage(CMMeshParsingErrorType::DESCRIPTOR_INDEX_COUNT_READ_FAILED_RETRIEVAL);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_VERTEX_BYTE_STRIDE_AND_HINT_READ_FAILED_RETRIEVAL =
		MeshParsingErrorMessage(CMMeshParsingErrorType::DESCRIPTOR_VERTEX_BYTE_STRIDE_AND_HINT_READ_FAILED_RETRIEVAL);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_INDEX_BYTE_STRIDE_AND_HINT_READ_FAILED_RETRIEVAL =
		MeshParsingErrorMessage(CMMeshParsingErrorType::DESCRIPTOR_INDEX_BYTE_STRIDE_AND_HINT_READ_FAILED_RETRIEVAL);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_ATTRIBUTES_READ_FAILED_RETRIEVAL =
		MeshParsingErrorMessage(CMMeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_READ_FAILED_RETRIEVAL);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_ATTRIBUTES_NAME_READ_FAILED_RETRIEVAL =
		MeshParsingErrorMessage(CMMeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_NAME_READ_FAILED_RETRIEVAL);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_ATTRIBUTES_BYTE_OFFSET_READ_FAILED_RETRIEVAL =
		MeshParsingErrorMessage(CMMeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_BYTE_OFFSET_READ_FAILED_RETRIEVAL);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_VERTEX_COUNT_PARSE_ZERO =
		MeshParsingErrorMessage(CMMeshParsingErrorType::DESCRIPTOR_VERTEX_COUNT_PARSE_ZERO);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_VERTEX_COUNT_PARSE_FAILED =
		MeshParsingErrorMessage(CMMeshParsingErrorType::DESCRIPTOR_VERTEX_COUNT_PARSE_FAILED);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_INDEX_COUNT_PARSE_ZERO =
		MeshParsingErrorMessage(CMMeshParsingErrorType::DESCRIPTOR_INDEX_COUNT_PARSE_ZERO);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_INDEX_COUNT_PARSE_FAILED =
		MeshParsingErrorMessage(CMMeshParsingErrorType::DESCRIPTOR_INDEX_COUNT_PARSE_FAILED);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_VERTEX_BYTE_STRIDE_PARSE_ZERO =
		MeshParsingErrorMessage(CMMeshParsingErrorType::DESCRIPTOR_VERTEX_BYTE_STRIDE_PARSE_ZERO);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_VERTEX_BYTE_STRIDE_AND_HINT_PARSE_FAILED =
		MeshParsingErrorMessage(CMMeshParsingErrorType::DESCRIPTOR_VERTEX_BYTE_STRIDE_AND_HINT_PARSE_FAILED);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_VERTEX_BYTE_STRIDE_HINT_PARSE_FAILED = 
		MeshParsingErrorMessage(CMMeshParsingErrorType::DESCRIPTOR_VERTEX_BYTE_STRIDE_HINT_PARSE_FAILED);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_VERTEX_BYTE_STRIDE_HINT_PARSE_INVALID =
		MeshParsingErrorMessage(CMMeshParsingErrorType::DESCRIPTOR_VERTEX_BYTE_STRIDE_HINT_PARSE_INVALID);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_INDEX_BYTE_STRIDE_PARSE_ZERO =
		MeshParsingErrorMessage(CMMeshParsingErrorType::DESCRIPTOR_INDEX_BYTE_STRIDE_PARSE_ZERO);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_INDEX_BYTE_STRIDE_AND_HINT_PARSE_FAILED =
		MeshParsingErrorMessage(CMMeshParsingErrorType::DESCRIPTOR_INDEX_BYTE_STRIDE_AND_HINT_PARSE_FAILED);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_INDEX_BYTE_STRIDE_HINT_PARSE_INVALID =
		MeshParsingErrorMessage(CMMeshParsingErrorType::DESCRIPTOR_INDEX_BYTE_STRIDE_HINT_PARSE_INVALID);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_INDEX_BYTE_STRIDE_HINT_PARSE_FAILED =
		MeshParsingErrorMessage(CMMeshParsingErrorType::DESCRIPTOR_INDEX_BYTE_STRIDE_HINT_PARSE_FAILED);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_ATTRIBUTES_PARSE_ABSENT =
		MeshParsingErrorMessage(CMMeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_PARSE_ABSENT);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_ATTRIBUTES_PARSE_NON_SEQUENCE =
		MeshParsingErrorMessage(CMMeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_PARSE_NON_SEQUENCE);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_ATTRIBUTES_TYPE_HINT_PARSE_FAILED =
		MeshParsingErrorMessage(CMMeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_TYPE_HINT_PARSE_FAILED);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_ATTRIBUTES_TYPE_HINT_PARSE_INVALID =
		MeshParsingErrorMessage(CMMeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_TYPE_HINT_PARSE_INVALID);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_ATTRIBUTES_BYTE_OFFSET_PARSE_FAILED =
		MeshParsingErrorMessage(CMMeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_BYTE_OFFSET_PARSE_FAILED);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_ATTRIBUTES_BYTE_OFFSET_PARSE_INVALID_FIRST =
		MeshParsingErrorMessage(CMMeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_BYTE_OFFSET_PARSE_INVALID_FIRST);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_BINARY_BEGIN_BYTE_POS_INVALID =
		MeshParsingErrorMessage(CMMeshParsingErrorType::DESCRIPTOR_BINARY_BEGIN_BYTE_POS_INVALID);

	inline constexpr std::wstring_view G_ERMW_DATA_READ_FAILED_RETRIEVAL =
		MeshParsingErrorMessage(CMMeshParsingErrorType::DATA_READ_FAILED_RETRIEVAL);

	inline constexpr std::wstring_view G_ERMW_DATA_VERTICES_READ_FAILED_RETRIEVAL =
		MeshParsingErrorMessage(CMMeshParsingErrorType::DATA_VERTICES_READ_FAILED_RETRIEVAL);

	inline constexpr std::wstring_view G_ERMW_DATA_INDICES_READ_FAILED_RETRIEVAL =
		MeshParsingErrorMessage(CMMeshParsingErrorType::DATA_INDICES_READ_FAILED_RETRIEVAL);

	inline constexpr std::wstring_view G_ERMW_DATA_VERTICES_PARSE_FAILED =
		MeshParsingErrorMessage(CMMeshParsingErrorType::DATA_VERTICES_PARSE_FAILED);

	inline constexpr std::wstring_view G_ERMW_DATA_INDICES_PARSE_FAILED =
		MeshParsingErrorMessage(CMMeshParsingErrorType::DATA_INDICES_PARSE_FAILED);
#pragma endregion

	class CMMeshSerializer
	{
	public:
		CMMeshSerializer(CMCommon::CMLoggerWide& logger) noexcept;
		~CMMeshSerializer() = default;
	public:
		[[nodiscard]] bool SaveMesh(const std::filesystem::path& filePath, const CMMeshData& meshData) noexcept;

		void LoadMesh(const std::filesystem::path& filePath, CMMeshData& outData) noexcept;

		void EnforceValidMeshDescriptor(const CMMeshDescriptor& descriptor, const std::filesystem::path& filePath) noexcept;
	private:
		void ParseNodes(
			CMMeshData& outData,
			const YAML::Node& root,
			const std::filesystem::path& filePath
		) noexcept;

		void LoadMeshDescriptor(
			CMMeshDescriptor& outDescriptor,
			const std::filesystem::path& filePath,
			const YAML::Node& meshNameNode,
			const YAML::Node& vertexCountNode,
			const YAML::Node& indexCountNode,
			const YAML::Node& vertexByteStrideNode,
			const YAML::Node& indexByteStrideNode,
			const YAML::Node& vertexByteStrideHintNode,
			const YAML::Node& indexByteStrideHintNode,
			const YAML::Node& vertexAttributesNode
		) noexcept;

		void LoadMeshData(
			CMMeshData& outMeshData,
			const std::wstring& filePathWStr,
			const YAML::Node& verticesNode,
			const YAML::Node& indicesNode
		) noexcept;

		void ParseVertexStrideNodes(
			CMMeshDescriptor& outDescriptor,
			const std::filesystem::path& filePath,
			const YAML::Node& vertexStrideNode,
			const YAML::Node& vertexStrideHintNode
		) noexcept;

		void ParseIndexStrideNodes(
			CMMeshDescriptor& outDescriptor,
			const std::filesystem::path& filePath,
			const YAML::Node& indexStrideNode,
			const YAML::Node& indexStrideHintNode
		) noexcept;

		void ParseAttributesNode(
			CMMeshDescriptor& outDescriptor,
			const std::filesystem::path& filePath,
			const YAML::Node& attributesNode
		) noexcept;

		template <typename Ty>
		[[nodiscard]] bool TryParseNode(const YAML::Node& node, Ty& outData);
	private:
		CMCommon::CMLoggerWide& m_Logger;
	};

	template <typename Ty>
	[[nodiscard]] bool CMMeshSerializer::TryParseNode(const YAML::Node& node, Ty& outData)
	{
		/* TODO:
		 *   Change this to parsing the type manually via the node's string
		 *     to avoid exceptions.
		 */
		try
		{
			outData = node.as<Ty>();
		}
		catch (const YAML::Exception& ex)
		{
			return false;
		}

		return true;
	}
}

namespace YAML
{
	template <>
	struct convert<CMEngine::Asset::CMVertexAttribute>
	{
		static Node encode(const CMEngine::Asset::CMVertexAttribute& attributeRef)
		{
			Node node;

			node["Name"] = attributeRef.Name;
			node["TypeHint"] = CMEngine::TypeHintToString(attributeRef.TypeHint);
			node["ByteOffset"] = attributeRef.ByteOffset;

			return node;
		}
	};

	template <>
	struct convert<CMEngine::Asset::CMMeshDescriptor>
	{
		static Node encode(const CMEngine::Asset::CMMeshDescriptor& desc)
		{
			Node sub;

			sub["MeshName"] = desc.MeshName;
			sub["VertexCount"] = desc.VertexCount;
			sub["IndexCount"] = desc.IndexCount;
			sub["VertexByteStride"] = desc.VertexByteStride;
			sub["IndexByteStride"] = desc.IndexByteStride;
			sub["VertexByteStrideHint"] = CMEngine::TypeHintToString(desc.VertexByteStrideHint);
			sub["IndexByteStrideHint"] = CMEngine::TypeHintToString(desc.IndexByteStrideHint);

			Node attributesNode(NodeType::Sequence);

			for (const auto& attr : desc.Attributes)
				attributesNode.push_back(convert<CMEngine::Asset::CMVertexAttribute>::encode(attr));

			sub["VertexAttributes"] = attributesNode;

			return sub;
		}
	};

	template <>
	struct convert<CMEngine::Asset::CMMeshData>
	{
		static Node encode(const CMEngine::Asset::CMMeshData& data)
		{
			Node root;
			Node descriptor = convert<CMEngine::Asset::CMMeshDescriptor>::encode(data.Descriptor);
			Node dataNode;
		
			YAML::Binary verticesBinary(
				reinterpret_cast<const uint8_t*>(data.VertexData.data()),
				data.VertexData.size()
			);

			YAML::Binary indicesBinary(
				reinterpret_cast<const uint8_t*>(data.IndexData.data()),
				data.VertexData.size()
			);

			dataNode["Vertices"] = verticesBinary;
			dataNode["Indices"] = indicesBinary;
			
			root["CMSerializedMesh"]["Descriptor"] = descriptor;
			root["CMSerializedMesh"]["Data"] = dataNode;

			return root;
		}
	};
}