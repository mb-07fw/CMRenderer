#include "CME_PCH.hpp"
#include "CME_MeshSerializer.hpp"
#include "CMC_Paths.hpp"

namespace CMEngine::Asset
{
	CMMeshSerializer::CMMeshSerializer(CMCommon::CMLoggerWide& logger) noexcept
		: m_Logger(logger)
	{
	}

	[[nodiscard]] bool CMMeshSerializer::SaveMesh(const std::filesystem::path& filePath, const CMMeshData& meshData) noexcept
	{
		YAML::Node root = YAML::convert<CMMeshData>::encode(meshData);

		std::ofstream oStream(filePath);

		if (!oStream)
			return false;

		oStream << root << '\n';
		oStream.flush();

		return true;
	}

	void CMMeshSerializer::LoadMesh(const std::filesystem::path& filePath, CMMeshData& outData) noexcept
	{
		YAML::Node root = YAML::LoadFile(filePath.string());

		ParseNodes(outData, root, filePath);
	}

	void CMMeshSerializer::EnforceValidMeshDescriptor(const CMMeshDescriptor& descriptor, const std::filesystem::path& filePath) noexcept
	{
		constexpr std::wstring_view FuncTag = L"CMMeshSerializer [EnforceValidMeshDescriptor] | ";

		std::wstring filePathWStr = filePath;

		m_Logger.LogFatalNLFormattedIf(
			descriptor.VertexCount == 0,
			FuncTag,
			G_ERMW_DESCRIPTOR_VERTEX_COUNT_PARSE_ZERO,
			filePathWStr
		);

		m_Logger.LogFatalNLFormattedIf(
			descriptor.IndexCount == 0,
			FuncTag,
			G_ERMW_DESCRIPTOR_INDEX_COUNT_PARSE_ZERO,
			filePathWStr
		);

		m_Logger.LogFatalNLFormattedIf(
			descriptor.VertexByteStride == 0,
			FuncTag,
			G_ERMW_DESCRIPTOR_VERTEX_BYTE_STRIDE_PARSE_ZERO,
			filePathWStr
		);

		m_Logger.LogFatalNLFormattedIf(
			descriptor.IndexByteStride == 0,
			FuncTag,
			G_ERMW_DESCRIPTOR_INDEX_BYTE_STRIDE_PARSE_ZERO,
			filePathWStr
		);

		m_Logger.LogFatalNLFormattedIf(
			descriptor.VertexByteStrideHint == CMTypeHint::INVALID,
			FuncTag,
			G_ERMW_DESCRIPTOR_VERTEX_BYTE_STRIDE_HINT_PARSE_INVALID,
			filePathWStr
		);

		m_Logger.LogFatalNLFormattedIf(
			descriptor.IndexByteStrideHint == CMTypeHint::INVALID,
			FuncTag,
			G_ERMW_DESCRIPTOR_INDEX_BYTE_STRIDE_HINT_PARSE_INVALID,
			filePathWStr
		);

		m_Logger.LogFatalNLVariadicIf(
			descriptor.Attributes.empty(),
			FuncTag,
			G_ERMW_DESCRIPTOR_ATTRIBUTES_PARSE_ABSENT,
			filePathWStr
		);

		size_t currentAttribute = 1;
		for (const CMVertexAttribute& vertexAttribute : descriptor.Attributes)
		{
			if (currentAttribute == 1)
				m_Logger.LogFatalNLFormattedIf(
					vertexAttribute.ByteOffset != 0,
					FuncTag,
					G_ERMW_DESCRIPTOR_ATTRIBUTES_BYTE_OFFSET_PARSE_INVALID_FIRST,
					vertexAttribute.ByteOffset, vertexAttribute.ByteOffset,
					filePathWStr
				);

			currentAttribute++;
		}

		m_Logger.LogFatalNLFormattedIf(
			descriptor.BinaryBeginBytePos == CMCommon::Constants::U32_MAX_VALUE,
			FuncTag,
			G_ERMW_DESCRIPTOR_BINARY_BEGIN_BYTE_POS_INVALID,
			descriptor.BinaryBeginBytePos,
			filePathWStr
		);
	}

	void CMMeshSerializer::ParseNodes(
		CMMeshData& outData,
		const YAML::Node& root,
		const std::filesystem::path& filePath
	) noexcept
	{
		constexpr std::wstring_view FuncTag = L"CMMeshSerializer [ParseNodes] | ";

		std::wstring filePathWStr = filePath.wstring();

		m_Logger.LogFatalNLFormattedIf(
			!root,
			FuncTag,
			G_ERMW_FILE_ROOT_READ_FAILED_RETRIEVAL,
			filePathWStr
		);

		YAML::Node meshNode = root["CMSerializedMesh"];

		m_Logger.LogFatalNLFormattedIf(
			!meshNode,
			FuncTag,
			G_ERMW_MESH_ROOT_READ_FAILED_RETRIEVAL,
			filePathWStr
		);

		YAML::Node descriptorNode = meshNode["Descriptor"];
		YAML::Node dataNode = meshNode["Data"];

		m_Logger.LogFatalNLFormattedIf(
			!descriptorNode,
			FuncTag,
			G_ERMW_DESCRIPTOR_READ_FAILED_RETRIEVAL,
			filePathWStr
		);

		m_Logger.LogFatalNLFormattedIf(
			!dataNode,
			FuncTag,
			G_ERMW_DATA_READ_FAILED_RETRIEVAL,
			filePathWStr
		);

		YAML::Node meshNameNode = descriptorNode["MeshName"];
		YAML::Node vertexCountNode = descriptorNode["VertexCount"];
		YAML::Node indexCountNode = descriptorNode["IndexCount"];
		YAML::Node vertexByteStrideNode = descriptorNode["VertexByteStride"];
		YAML::Node indexByteStrideNode = descriptorNode["IndexByteStride"];
		YAML::Node vertexByteStrideHintNode = descriptorNode["VertexByteStrideHint"];
		YAML::Node indexByteStrideHintNode = descriptorNode["IndexByteStrideHint"];
		YAML::Node vertexAttributesNode = descriptorNode["VertexAttributes"];

		YAML::Node verticesNode = dataNode["Vertices"];
		YAML::Node indicesNode = dataNode["Indices"];

		m_Logger.LogFatalNLFormattedIf(
			!meshNameNode,
			FuncTag,
			G_ERMW_DESCRIPTOR_MESH_NAME_READ_FAILED_RETRIEVAL,
			filePathWStr
		);

		m_Logger.LogFatalNLFormattedIf(
			!vertexCountNode,
			FuncTag,
			G_ERMW_DESCRIPTOR_VERTEX_COUNT_READ_FAILED_RETRIEVAL,
			filePathWStr
		);

		m_Logger.LogFatalNLFormattedIf(
			!indexCountNode,
			FuncTag,
			G_ERMW_DESCRIPTOR_VERTEX_COUNT_READ_FAILED_RETRIEVAL,
			filePathWStr
		);

		bool foundValidVertexStride = vertexByteStrideNode || vertexByteStrideHintNode;
		bool foundValidIndexStride = indexByteStrideNode || indexByteStrideHintNode;

		m_Logger.LogFatalNLFormattedIf(
			!foundValidVertexStride,
			FuncTag,
			G_ERMW_DESCRIPTOR_VERTEX_BYTE_STRIDE_AND_HINT_READ_FAILED_RETRIEVAL,
			filePathWStr
		);

		m_Logger.LogFatalNLFormattedIf(
			!foundValidIndexStride,
			FuncTag,
			G_ERMW_DESCRIPTOR_INDEX_BYTE_STRIDE_AND_HINT_READ_FAILED_RETRIEVAL,
			filePathWStr
		);

		m_Logger.LogFatalNLFormattedIf(
			!vertexAttributesNode,
			FuncTag,
			G_ERMW_DESCRIPTOR_ATTRIBUTES_READ_FAILED_RETRIEVAL,
			filePathWStr
		);

		m_Logger.LogFatalNLFormattedIf(
			!verticesNode,
			FuncTag,
			G_ERMW_DATA_VERTICES_READ_FAILED_RETRIEVAL,
			filePathWStr
		);
		
		m_Logger.LogFatalNLFormattedIf(
			!indicesNode,
			FuncTag,
			G_ERMW_DATA_INDICES_READ_FAILED_RETRIEVAL,
			filePathWStr
		);

		LoadMeshDescriptor(
			outData.Descriptor,
			filePath,
			meshNameNode,
			vertexCountNode,
			indexCountNode,
			vertexByteStrideNode,
			indexByteStrideNode,
			vertexByteStrideHintNode,
			indexByteStrideHintNode,
			vertexAttributesNode
		);

		LoadMeshData(outData, filePathWStr, verticesNode, indicesNode);
	}

	void CMMeshSerializer::LoadMeshDescriptor(
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
	) noexcept
	{
		constexpr std::wstring_view FuncTag = L"CMMeshSerializer [LoadMeshDescriptor] | ";

		std::wstring filePathWStr = filePath.wstring();

		outDescriptor.MeshName = meshNameNode.as<std::string>();

		/* TODO: Manually parse data via node strings
		 *       to avoid exceptions in TryParseNode.
		 */
		m_Logger.LogFatalNLFormattedIf(
			!TryParseNode(vertexCountNode, outDescriptor.VertexCount),
			FuncTag,
			G_ERMW_DESCRIPTOR_VERTEX_COUNT_PARSE_FAILED,
			filePathWStr
		);

		m_Logger.LogFatalNLFormattedIf(
			!TryParseNode(indexCountNode, outDescriptor.IndexCount),
			FuncTag,
			G_ERMW_DESCRIPTOR_INDEX_COUNT_PARSE_FAILED,
			filePathWStr
		);

		ParseVertexStrideNodes(
			outDescriptor,
			filePath,
			vertexByteStrideNode,
			vertexByteStrideHintNode
		);

		ParseIndexStrideNodes(
			outDescriptor,
			filePath,
			indexByteStrideNode,
			indexByteStrideHintNode
		);

		ParseAttributesNode(outDescriptor, filePath, vertexAttributesNode);
	}

	void CMMeshSerializer::LoadMeshData(
		CMMeshData& outMeshData,
		const std::wstring& filePathWStr,
		const YAML::Node& verticesNode,
		const YAML::Node& indicesNode
	) noexcept
	{
		constexpr std::wstring_view FuncTag = L"CMMeshSerializer [LoadMeshData] | ";

		YAML::Binary verticesBinary;
		YAML::Binary indicesBinary;

		m_Logger.LogFatalNLFormattedIf(
			!TryParseNode<YAML::Binary>(verticesNode, verticesBinary),
			FuncTag,
			G_ERMW_DATA_VERTICES_PARSE_FAILED,
			filePathWStr
		);

		m_Logger.LogFatalNLFormattedIf(
			!TryParseNode<YAML::Binary>(indicesNode, indicesBinary),
			FuncTag,
			G_ERMW_DATA_INDICES_PARSE_FAILED,
			filePathWStr
		);

		outMeshData.VertexData.reserve(verticesBinary.size());
		outMeshData.IndexData.reserve(indicesBinary.size());

		const std::byte* pVertices = reinterpret_cast<const std::byte*>(verticesBinary.data());
		const std::byte* pIndices = reinterpret_cast<const std::byte*>(indicesBinary.data());

		outMeshData.VertexData.assign(pVertices, pVertices + verticesBinary.size());
		outMeshData.IndexData.assign(pIndices, pIndices + indicesBinary.size());
	}

	void CMMeshSerializer::ParseVertexStrideNodes(
		CMMeshDescriptor& outDescriptor,
		const std::filesystem::path& filePath,
		const YAML::Node& vertexStrideNode,
		const YAML::Node& vertexStrideHintNode
	) noexcept
	{
		constexpr std::wstring_view FuncTag = L"CMMeshSerializer [ParseVertexStrideNodes] | ";

		std::wstring filePathWStr = filePath.wstring();

		if (vertexStrideHintNode.IsDefined())
		{
			std::string hintStr = vertexStrideHintNode.as<std::string>();

			bool parsed = false;
			outDescriptor.VertexByteStrideHint = StringToTypeHint(hintStr, parsed);

			m_Logger.LogFatalNLFormattedIf(
				!parsed,
				FuncTag,
				G_ERMW_DESCRIPTOR_VERTEX_BYTE_STRIDE_HINT_PARSE_FAILED,
				filePathWStr
			);

			m_Logger.LogFatalNLFormattedIf(
				outDescriptor.VertexByteStrideHint == CMTypeHint::INVALID,
				FuncTag,
				G_ERMW_DESCRIPTOR_VERTEX_BYTE_STRIDE_HINT_PARSE_INVALID,
				filePathWStr
			);

			outDescriptor.VertexByteStride = ToByteSize(outDescriptor.VertexByteStrideHint);

			if (outDescriptor.VertexByteStride != 0)
				return;
		}

		outDescriptor.VertexByteStrideHint = CMTypeHint::NONE;

		m_Logger.LogFatalNLFormattedIf(
			!TryParseNode(
				vertexStrideNode,
				outDescriptor.VertexByteStride
			),
			FuncTag,
			G_ERMW_DESCRIPTOR_VERTEX_BYTE_STRIDE_AND_HINT_PARSE_FAILED,
			filePathWStr
		);
	}

	void CMMeshSerializer::ParseIndexStrideNodes(
		CMMeshDescriptor& outDescriptor,
		const std::filesystem::path& filePath,
		const YAML::Node& indexStrideNode,
		const YAML::Node& indexStrideHintNode
	) noexcept
	{
		constexpr std::wstring_view FuncTag = L"CMMeshSerializer [ParseIndexStrideNodes] | ";

		std::wstring filePathWStr = filePath.wstring();

		if (indexStrideHintNode.IsDefined())
		{
			std::string hintStr = indexStrideHintNode.as<std::string>();

			bool parsed = false;
			outDescriptor.IndexByteStrideHint = StringToTypeHint(hintStr, parsed);

			m_Logger.LogFatalNLFormattedIf(
				!parsed,
				FuncTag,
				G_ERMW_DESCRIPTOR_INDEX_BYTE_STRIDE_HINT_PARSE_FAILED,
				filePathWStr
			);

			m_Logger.LogFatalNLFormattedIf(
				outDescriptor.IndexByteStrideHint == CMTypeHint::INVALID,
				FuncTag,
				G_ERMW_DESCRIPTOR_INDEX_BYTE_STRIDE_HINT_PARSE_INVALID,
				filePathWStr
			);

			outDescriptor.IndexByteStride = ToByteSize(outDescriptor.IndexByteStrideHint);
			
			if (outDescriptor.IndexByteStride != 0)
				return;
		}

		outDescriptor.IndexByteStrideHint = CMTypeHint::NONE;

		m_Logger.LogFatalNLFormattedIf(
			!TryParseNode(
				indexStrideNode,
				outDescriptor.IndexByteStride
			),
			FuncTag,
			G_ERMW_DESCRIPTOR_INDEX_BYTE_STRIDE_AND_HINT_PARSE_FAILED,
			filePathWStr
		);
	}

	void CMMeshSerializer::ParseAttributesNode(
		CMMeshDescriptor& outDescriptor,
		const std::filesystem::path& filePath,
		const YAML::Node& attributesNode
	) noexcept
	{
		constexpr std::wstring_view FuncTag = L"CMMeshSerializer [ParseAttributesNode] |";

		std::wstring filePathWStr = filePath.wstring();

		/* TODO: Utilize YAML::convert (yaml namespace) of types to make parsing easier. */
		m_Logger.LogFatalNLFormattedIf(
			!attributesNode.IsSequence(),
			FuncTag,
			G_ERMW_DESCRIPTOR_ATTRIBUTES_PARSE_NON_SEQUENCE,
			filePathWStr
		);

		outDescriptor.Attributes.reserve(attributesNode.size());

		uint32_t currentAttribute = 1;
		for (const auto& node : attributesNode)
		{
			YAML::Node nameNode = node["Name"];
			YAML::Node typeHintNode = node["TypeHint"];
			YAML::Node byteOffsetNode = node["ByteOffset"];

			m_Logger.LogFatalNLFormattedIf(
				!nameNode, 
				FuncTag,
				G_ERMW_DESCRIPTOR_ATTRIBUTES_NAME_READ_FAILED_RETRIEVAL,
				currentAttribute, filePathWStr
			);

			m_Logger.LogFatalNLFormattedIf(
				!byteOffsetNode,
				FuncTag,
				G_ERMW_DESCRIPTOR_ATTRIBUTES_BYTE_OFFSET_READ_FAILED_RETRIEVAL,
				currentAttribute, filePathWStr
			);

			std::string name = nameNode.as<std::string>();
			uint32_t byteOffset = 0;

			m_Logger.LogFatalNLFormattedIf(
				!TryParseNode(byteOffsetNode, byteOffset),
				FuncTag,
				G_ERMW_DESCRIPTOR_ATTRIBUTES_BYTE_OFFSET_PARSE_FAILED,
				currentAttribute, filePathWStr
			);

			/* A TypeHint node being absent is perfectly legal, 
			 * and implies the data is not compatible with a preset CMTypeHint.
			 */
			if (!typeHintNode)
			{
				outDescriptor.Attributes.emplace_back(name, CMTypeHint::NONE, byteOffset);
				continue;
			}

			std::string typeHintStr = typeHintNode.as<std::string>();
			bool parsed = false;
			CMTypeHint typeHint = StringToTypeHint(typeHintStr, parsed);

			m_Logger.LogFatalNLFormattedIf(
				!parsed,
				FuncTag,
				G_ERMW_DESCRIPTOR_ATTRIBUTES_TYPE_HINT_PARSE_FAILED,
				currentAttribute, filePathWStr
			);

			m_Logger.LogFatalNLFormattedIf(
				typeHint == CMTypeHint::INVALID,
				FuncTag,
				G_ERMW_DESCRIPTOR_ATTRIBUTES_TYPE_HINT_PARSE_INVALID,
				currentAttribute, filePathWStr
			);

			outDescriptor.Attributes.emplace_back(name, typeHint, byteOffset);
			++currentAttribute;
		}
	}
}