#pragma once

#include <string>
#include <string_view>
#include <filesystem>
#include <type_traits>
#include <array>

#include "Core/MeshData.hpp"
#include "Common/Logger.hpp"
#include "Common/BinaryStream.hpp"

#include "yaml-cpp/yaml.h"

namespace CMEngine::Core::Asset
{
#pragma region Parsing Errors
	enum class MeshParsingErrorType : int8_t
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
	
	/* Technically there are MeshParingErrorType::TOTAL + 1 elements, but only MeshParsingErrorType::TOTAL messages. */
	inline constexpr std::array<std::wstring_view, static_cast<size_t>(MeshParsingErrorType::TOTAL)> G_MESH_PARSING_ERROR_MESSAGES = { {
		/* MeshParsingErrorType::FILE_READ_FAILED_OPEN */
		{ L"Failed to open .yaml file : `{}`. Does it exist?" },

		/* MeshParsingErrorType::FILE_ROOT_READ_FAILED_RETRIEVAL */
		{ L"Failed to retrieve root yaml node of mesh file : `{}`." },

		/* MeshParsingErrorType::MESH_ROOT_READ_FAILED_RETRIEVAL */
		{ L"Failed to retrieve SerializedMesh node of mesh file `{}`." },

		/* MeshParsingErrorType::DESCRIPTOR_READ_FAILED_RETRIEVAL */
		{ L"Failed to retrieve Descriptor node of mesh descriptor : `{}`." },

		/* MeshParsingErrorType::DESCRIPTOR_MESH_NAME_READ_FAILED_RETRIEVAL */
		{ L"Failed to retrieve MeshName node of mesh descriptor : `{}`." },

		/* MeshParsingErrorType::DESCRIPTOR_VERTEX_COUNT_READ_FAILED_RETRIEVAL */
		{ L"Failed to retrieve VertexCount node of mesh descriptor : `{}`." },

		/* MeshParsingErrorType::DESCRIPTOR_INDEX_COUNT_READ_FAILED_RETRIEVAL */
		{ L"Failed to retrieve IndexCount node of mesh descriptor : `{}`." },

		/* MeshParsingErrorType::DESCRIPTOR_VERTEX_BYTE_STRIDE_AND_HINT_READ_FAILED_RETRIEVAL */
		{ L"Failed to retrieve valid vertex byte stride from VertexByteStride "
		  L"or VertexByteStrideHint nodes from mesh descriptor : `{}`."
		},

		/* MeshParsingErrorType::DESCRIPTOR_INDEX_BYTE_STRIDE_AND_HINT_READ_FAILED_RETRIEVAL */
		{ L"Failed to retrieve valid index byte stride from IndexByteStride "
		  L"or IndexByteStrideHint nodes from mesh descriptor : `{}`."
		},

		/* MeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_READ_FAILED_RETRIEVAL */
		{ L"Failed to retrieve Attributes node from mesh descriptor : `{}`." },

		/* MeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_NAME_READ_FAILED_RETRIEVAL */
		{ L"Failed to retrieve Name node from attribute `{}`. Mesh descriptor : `{}`." },

		/* MeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_BYTE_OFFSET_READ_FAILED_RETRIEVAL */
		{ L"Failed to retrieve ByteOffset node from attribute `{}`. Mesh descriptor : `{}`." },

		/* MeshParsingErrorType::DESCRIPTOR_VERTEX_COUNT_PARSE_ZERO */
		{ L"Vertex count is 0. A serialized mesh should have vertices to serialize. Mesh descriptor : `{}`." },

		/* MeshParsingErrorType::DESCRIPTOR_VERTEX_COUNT_PARSE_FAILED */
		{ L"Failed to parse VertexCount node from mesh descriptor : `{}`." },

		/* MeshParsingErrorType::DESCRIPTOR_INDEX_COUNT_PARSE_ZERO */
		{ L"Index count is 0. A serialized mesh should have indices to serialize. Mesh descriptor : `{}`." },

		/* MeshParsingErrorType::DESCRIPTOR_INDEX_COUNT_PARSE_FAILED */
		{ L"Failed to parse IndexCount node from mesh descriptor : `{}`." },

		/* MeshParsingErrorType::DESCRIPTOR_VERTEX_BYTE_STRIDE_PARSE_ZERO */
		{ L"Vertex byte stride is 0. It is imperitive that a serialized mesh has "
		  L"a valid vertex byte stride defined in order to successfully parse it's binary data. "
		  L"Mesh descriptor : `{}`."
		},

		/* MeshParsingErrorType::DESCRIPTOR_VERTEX_BYTE_STRIDE_AND_HINT_PARSE_FAILED */
		{ L"Failed to parse VertexByteStride and no VertexByteStrideHint is "
		  L"defined, or VertexByteStrideHint is TypeHint::NONE (`none`). "
		  L"Mesh descriptor : `{}`."
		},

		/* MeshParsingErrorType::DESCRIPTOR_VERTEX_BYTE_STRIDE_HINT_PARSE_FAILED */
		{ L"Failed to parse VertexByteStrideHint node from mesh descriptor : `{}`. "
		  L"If VertexByteStride was intended to be prioritzed, VertexByteStrideHint "
		  L"should be TypeHint::NONE (`none` or left undefined in parsing)."
		},

		/* MeshParsingErrorType::DESCRIPTOR_VERTEX_BYTE_STRIDE_HINT_PARSE_INVALID */
		{ L"Vertex byte stride hint is TypeHint::INVALID (`invalid`). This implies that the parsed type "
		  L"hint had the value TypeHint::INVALID, which is illegal, instead of the abscence "
		  L"of a TypeHint being described by TypeHint::NONE. If VertexByteStride was intended "
		  L"to be prioritized, VertexByteStrideHint should be TypeHint::NONE (`none` or left undefined in parsing). "
		  L"Mesh descriptor : `{}`."
		},

		/* MeshParsingErrorType::DESCRIPTOR_INDEX_BYTE_STRIDE_PARSE_ZERO */
		{ L"Index byte stride is 0. It is imperitive that a serialized mesh has an index byte "
		  L"stride defined in order to successfully parse it's binary data. Mesh descriptor : `{}`."
		},

		/* MeshParsingErrorType::DESCRIPTOR_INDEX_BYTE_STRIDE_AND_HINT_PARSE_FAILED */
		{ L"Failed to parse IndexByteStride and no IndexByteStrideHint is defined, or IndexByteStrideHint is "
		  L"HintType::NONE. Mesh descriptor : `{}`."
	    },

		/* MeshParsingErrorType::DESCRIPTOR_INDEX_BYTE_STRIDE_HINT_PARSE_FAILED */
		{ L"Failed to parse IndexByteStrideHint node from mesh descriptor : `{}`. "
		  L"If VertexByteStride was intended to be prioritzed, VertexByteStrideHint "
		  L"should be TypeHint::NONE (`none` or left undefined in parsing)."
		},

		/* MeshParsingErrorType::DESCRIPTOR_INDEX_BYTE_STRIDE_HINT_PARSE_INVALID */
		{ L"Index byte stride hint is TypeHint::INVALID. This implies that the parsed type hint had the value "
		  L"TypeHint::INVALID, which is illegal, instead of the abscence of a TypeHint being described by "
		  L"TypeHint::NONE. If IndexByteStride was intended to be prioritized, IndexByteStrideHint should be "
		  L"TypeHint::NONE (`none` or left undefined in parsing). Mesh descriptor : `{}`."
		},
		
		/* MeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_PARSE_ABSENT */
		{ L"No vertex attributes are present. It is imperitive that a serialized mesh has vertex attributes "
		  L"defined in order to successfully parsed it's binary data. Mesh descriptor : `{}`."
		},

		/* MeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_PARSE_NON_SEQUENCE */
		{ L"VertexAttributes node isn't a sequence. This means the node doesn't contain a list of "
		  L"attributes which is illegal for a serialized mesh."
		},

		/* MeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_TYPE_HINT_PARSE_FAILED */
		{ L"Failed to parse a valid TypeHint from TypeHint node. If TypeHint of the VertexAttribute " 
		  L"was intended to be discarded due to the data type not being compatible, TypeHint should "
		  L"have the string value `none` or shouldn't be defined. Attribute `{}`, Mesh descriptor : `{}`."
		},

		/* MeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_TYPE_HINT_PARSE_INVALID */
		{ L"TypeHint node has the value TypeHint::INVALID. "
		  L"If TypeHint of the VertexAttribute was intended to be discarded due to the data type "
		  L"not being compatible with a TypeHint, TypeHint should have the string value `none` or shouldn't be defined. "
		  L"Attribute `{}`, Mesh descriptor : `{}`."
		},

		/* MeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_BYTE_OFFSET_PARSE_FAILED */
		{ L"Failed to parse a valid byte offset from ByteOffset node from attribute. `{}`. Mesh descriptor : `{}`." },

		/* MeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_BYTE_OFFSET_PARSE_INVALID_FIRST */
		{ L"The first vertex attribute should have a byte offset of 0. A byte offset of `{}` for the first attribute "
		  L"implies you want to skip `{}` bytes of the beginning of each vertex, which is most likely malformed. "
		  L"Mesh descriptor : `{}`."
		},

		/* MeshParsingErrorType::DESCRIPTOR_BINARY_BEGIN_BYTE_POS_INVALID */
		{ L"The mesh descriptor doesn't have a valid byte offset of which to read in it's "
		  L"binary data. Byte offset : `{}`, Mesh descriptor : `{}`."
		},

		/* MeshParsingErrorType::DATA_READ_FAILED_RETRIEVAL */
		{ L"Failed to retrieve Data node from mesh file : `{}`." },

		/* MeshParsingErrorType::DATA_VERTICES_READ_FAILED_RETRIEVAL */
		{ L"Failed to retrieve Vertices node from mesh file : `{}`." },

		/* MeshParsingErrorType::DATA_INDICES_READ_FAILED_RETRIEVAL */
		{ L"Failed to retrieve Indices node from mesh file : `{}`." },

		/* MeshParsingErrorType::DATA_VERTICES_PARSE_FAILED */
		{ L"Failed to parse binary from Vertices node from mesh file : `{}`." },

		/* MeshParsingErrorType::DATA_INDICES_PARSE_FAILED */
		{ L"Failed to parse binary from Indices node from mesh file : `{}`." },
	} };

	inline constexpr [[nodiscard]] int8_t MeshParsingErrorIndex(MeshParsingErrorType errorType) noexcept
	{
		static_assert(static_cast<size_t>(MeshParsingErrorType::TOTAL) == G_MESH_PARSING_ERROR_MESSAGES.size(),
			"Mismatch between error enum and message array.");

		return static_cast<int8_t>(errorType);
	}

	inline constexpr [[nodiscard]] std::wstring_view MeshParsingErrorMessage(MeshParsingErrorType errorType) noexcept
	{
		static_assert(static_cast<size_t>(MeshParsingErrorType::TOTAL) == G_MESH_PARSING_ERROR_MESSAGES.size(),
			"Mismatch between error enum and message array.");

		int8_t index = MeshParsingErrorIndex(errorType);

		if (index < 0 || index >= G_MESH_PARSING_ERROR_MESSAGES.size())
			return std::wstring_view(L"");

		return G_MESH_PARSING_ERROR_MESSAGES[index];
	}

	inline constexpr std::wstring_view G_ERMW_FILE_READ_FAILED_OPEN =
		MeshParsingErrorMessage(MeshParsingErrorType::FILE_READ_FAILED_OPEN);

	inline constexpr std::wstring_view G_ERMW_FILE_ROOT_READ_FAILED_RETRIEVAL =
		MeshParsingErrorMessage(MeshParsingErrorType::FILE_ROOT_READ_FAILED_RETRIEVAL);

	inline constexpr std::wstring_view G_ERMW_MESH_ROOT_READ_FAILED_RETRIEVAL =
		MeshParsingErrorMessage(MeshParsingErrorType::MESH_ROOT_READ_FAILED_RETRIEVAL);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_READ_FAILED_RETRIEVAL =
		MeshParsingErrorMessage(MeshParsingErrorType::DESCRIPTOR_READ_FAILED_RETRIEVAL);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_MESH_NAME_READ_FAILED_RETRIEVAL =
		MeshParsingErrorMessage(MeshParsingErrorType::DESCRIPTOR_MESH_NAME_READ_FAILED_RETRIEVAL);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_VERTEX_COUNT_READ_FAILED_RETRIEVAL =
		MeshParsingErrorMessage(MeshParsingErrorType::DESCRIPTOR_VERTEX_COUNT_READ_FAILED_RETRIEVAL);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_INDEX_COUNT_READ_FAILED_RETRIEVAL =
		MeshParsingErrorMessage(MeshParsingErrorType::DESCRIPTOR_INDEX_COUNT_READ_FAILED_RETRIEVAL);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_VERTEX_BYTE_STRIDE_AND_HINT_READ_FAILED_RETRIEVAL =
		MeshParsingErrorMessage(MeshParsingErrorType::DESCRIPTOR_VERTEX_BYTE_STRIDE_AND_HINT_READ_FAILED_RETRIEVAL);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_INDEX_BYTE_STRIDE_AND_HINT_READ_FAILED_RETRIEVAL =
		MeshParsingErrorMessage(MeshParsingErrorType::DESCRIPTOR_INDEX_BYTE_STRIDE_AND_HINT_READ_FAILED_RETRIEVAL);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_ATTRIBUTES_READ_FAILED_RETRIEVAL =
		MeshParsingErrorMessage(MeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_READ_FAILED_RETRIEVAL);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_ATTRIBUTES_NAME_READ_FAILED_RETRIEVAL =
		MeshParsingErrorMessage(MeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_NAME_READ_FAILED_RETRIEVAL);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_ATTRIBUTES_BYTE_OFFSET_READ_FAILED_RETRIEVAL =
		MeshParsingErrorMessage(MeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_BYTE_OFFSET_READ_FAILED_RETRIEVAL);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_VERTEX_COUNT_PARSE_ZERO =
		MeshParsingErrorMessage(MeshParsingErrorType::DESCRIPTOR_VERTEX_COUNT_PARSE_ZERO);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_VERTEX_COUNT_PARSE_FAILED =
		MeshParsingErrorMessage(MeshParsingErrorType::DESCRIPTOR_VERTEX_COUNT_PARSE_FAILED);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_INDEX_COUNT_PARSE_ZERO =
		MeshParsingErrorMessage(MeshParsingErrorType::DESCRIPTOR_INDEX_COUNT_PARSE_ZERO);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_INDEX_COUNT_PARSE_FAILED =
		MeshParsingErrorMessage(MeshParsingErrorType::DESCRIPTOR_INDEX_COUNT_PARSE_FAILED);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_VERTEX_BYTE_STRIDE_PARSE_ZERO =
		MeshParsingErrorMessage(MeshParsingErrorType::DESCRIPTOR_VERTEX_BYTE_STRIDE_PARSE_ZERO);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_VERTEX_BYTE_STRIDE_AND_HINT_PARSE_FAILED =
		MeshParsingErrorMessage(MeshParsingErrorType::DESCRIPTOR_VERTEX_BYTE_STRIDE_AND_HINT_PARSE_FAILED);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_VERTEX_BYTE_STRIDE_HINT_PARSE_FAILED = 
		MeshParsingErrorMessage(MeshParsingErrorType::DESCRIPTOR_VERTEX_BYTE_STRIDE_HINT_PARSE_FAILED);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_VERTEX_BYTE_STRIDE_HINT_PARSE_INVALID =
		MeshParsingErrorMessage(MeshParsingErrorType::DESCRIPTOR_VERTEX_BYTE_STRIDE_HINT_PARSE_INVALID);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_INDEX_BYTE_STRIDE_PARSE_ZERO =
		MeshParsingErrorMessage(MeshParsingErrorType::DESCRIPTOR_INDEX_BYTE_STRIDE_PARSE_ZERO);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_INDEX_BYTE_STRIDE_AND_HINT_PARSE_FAILED =
		MeshParsingErrorMessage(MeshParsingErrorType::DESCRIPTOR_INDEX_BYTE_STRIDE_AND_HINT_PARSE_FAILED);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_INDEX_BYTE_STRIDE_HINT_PARSE_INVALID =
		MeshParsingErrorMessage(MeshParsingErrorType::DESCRIPTOR_INDEX_BYTE_STRIDE_HINT_PARSE_INVALID);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_INDEX_BYTE_STRIDE_HINT_PARSE_FAILED =
		MeshParsingErrorMessage(MeshParsingErrorType::DESCRIPTOR_INDEX_BYTE_STRIDE_HINT_PARSE_FAILED);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_ATTRIBUTES_PARSE_ABSENT =
		MeshParsingErrorMessage(MeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_PARSE_ABSENT);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_ATTRIBUTES_PARSE_NON_SEQUENCE =
		MeshParsingErrorMessage(MeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_PARSE_NON_SEQUENCE);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_ATTRIBUTES_TYPE_HINT_PARSE_FAILED =
		MeshParsingErrorMessage(MeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_TYPE_HINT_PARSE_FAILED);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_ATTRIBUTES_TYPE_HINT_PARSE_INVALID =
		MeshParsingErrorMessage(MeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_TYPE_HINT_PARSE_INVALID);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_ATTRIBUTES_BYTE_OFFSET_PARSE_FAILED =
		MeshParsingErrorMessage(MeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_BYTE_OFFSET_PARSE_FAILED);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_ATTRIBUTES_BYTE_OFFSET_PARSE_INVALID_FIRST =
		MeshParsingErrorMessage(MeshParsingErrorType::DESCRIPTOR_ATTRIBUTES_BYTE_OFFSET_PARSE_INVALID_FIRST);

	inline constexpr std::wstring_view G_ERMW_DESCRIPTOR_BINARY_BEGIN_BYTE_POS_INVALID =
		MeshParsingErrorMessage(MeshParsingErrorType::DESCRIPTOR_BINARY_BEGIN_BYTE_POS_INVALID);

	inline constexpr std::wstring_view G_ERMW_DATA_READ_FAILED_RETRIEVAL =
		MeshParsingErrorMessage(MeshParsingErrorType::DATA_READ_FAILED_RETRIEVAL);

	inline constexpr std::wstring_view G_ERMW_DATA_VERTICES_READ_FAILED_RETRIEVAL =
		MeshParsingErrorMessage(MeshParsingErrorType::DATA_VERTICES_READ_FAILED_RETRIEVAL);

	inline constexpr std::wstring_view G_ERMW_DATA_INDICES_READ_FAILED_RETRIEVAL =
		MeshParsingErrorMessage(MeshParsingErrorType::DATA_INDICES_READ_FAILED_RETRIEVAL);

	inline constexpr std::wstring_view G_ERMW_DATA_VERTICES_PARSE_FAILED =
		MeshParsingErrorMessage(MeshParsingErrorType::DATA_VERTICES_PARSE_FAILED);

	inline constexpr std::wstring_view G_ERMW_DATA_INDICES_PARSE_FAILED =
		MeshParsingErrorMessage(MeshParsingErrorType::DATA_INDICES_PARSE_FAILED);
#pragma endregion

	class MeshSerializer
	{
	public:
		MeshSerializer(Common::LoggerWide& logger) noexcept;
		~MeshSerializer() = default;
	public:
		[[nodiscard]] bool SaveMesh(const std::filesystem::path& filePath, const MeshData& meshData) noexcept;

		void LoadMesh(const std::filesystem::path& filePath, MeshData& outData) noexcept;

		void EnforceValidMeshDescriptor(const MeshDescriptor& descriptor, const std::filesystem::path& filePath) noexcept;
	private:
		void ParseNodes(
			MeshData& outData,
			const YAML::Node& root,
			const std::filesystem::path& filePath
		) noexcept;

		void LoadMeshDescriptor(
			MeshDescriptor& outDescriptor,
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
			MeshData& outMeshData,
			const std::wstring& filePathWStr,
			const YAML::Node& verticesNode,
			const YAML::Node& indicesNode
		) noexcept;

		void ParseVertexStrideNodes(
			MeshDescriptor& outDescriptor,
			const std::filesystem::path& filePath,
			const YAML::Node& vertexStrideNode,
			const YAML::Node& vertexStrideHintNode
		) noexcept;

		void ParseIndexStrideNodes(
			MeshDescriptor& outDescriptor,
			const std::filesystem::path& filePath,
			const YAML::Node& indexStrideNode,
			const YAML::Node& indexStrideHintNode
		) noexcept;

		void ParseAttributesNode(
			MeshDescriptor& outDescriptor,
			const std::filesystem::path& filePath,
			const YAML::Node& attributesNode
		) noexcept;

		template <typename Ty>
		[[nodiscard]] bool TryParseNode(const YAML::Node& node, Ty& outData);
	private:
		Common::LoggerWide& m_Logger;
	};

	template <typename Ty>
	[[nodiscard]] bool MeshSerializer::TryParseNode(const YAML::Node& node, Ty& outData)
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
	struct convert<CMEngine::Core::Asset::VertexAttribute>
	{
		static Node encode(const CMEngine::Core::Asset::VertexAttribute& attributeRef)
		{
			Node node;

			node["Name"] = attributeRef.Name;
			node["TypeHint"] = CMEngine::Core::TypeHintToString(attributeRef.TypeHint);
			node["ByteOffset"] = attributeRef.ByteOffset;

			return node;
		}
	};

	template <>
	struct convert<CMEngine::Core::Asset::MeshDescriptor>
	{
		static Node encode(const CMEngine::Core::Asset::MeshDescriptor& desc)
		{
			Node sub;

			sub["MeshName"] = desc.MeshName;
			sub["VertexCount"] = desc.VertexCount;
			sub["IndexCount"] = desc.IndexCount;
			sub["VertexByteStride"] = desc.VertexByteStride;
			sub["IndexByteStride"] = desc.IndexByteStride;
			sub["VertexByteStrideHint"] = CMEngine::Core::TypeHintToString(desc.VertexByteStrideHint);
			sub["IndexByteStrideHint"] = CMEngine::Core::TypeHintToString(desc.IndexByteStrideHint);

			Node attributesNode(NodeType::Sequence);

			for (const auto& attr : desc.Attributes)
				attributesNode.push_back(convert<CMEngine::Core::Asset::VertexAttribute>::encode(attr));

			sub["VertexAttributes"] = attributesNode;

			return sub;
		}
	};

	template <>
	struct convert<CMEngine::Core::Asset::MeshData>
	{
		static Node encode(const CMEngine::Core::Asset::MeshData& data)
		{
			Node root;
			Node descriptor = convert<CMEngine::Core::Asset::MeshDescriptor>::encode(data.Descriptor);
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
			
			root["SerializedMesh"]["Descriptor"] = descriptor;
			root["SerializedMesh"]["Data"] = dataNode;

			return root;
		}
	};
}