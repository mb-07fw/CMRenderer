#include "Core/CME_PCH.hpp"
#include "DirectX/CME_DXShaderLibrary.hpp"

namespace CMEngine::DirectXAPI::DX11
{
	DXShaderLibrary::DXShaderLibrary(CMCommon::CMLoggerWide& logger) noexcept
		: m_Logger(logger)
	{
		LoadShaders();

		m_Logger.LogInfoNL(L"DXShaderLibrary [()] | Constructed.");
	}

	DXShaderLibrary::~DXShaderLibrary() noexcept
	{
		if (m_Initialized)
			Shutdown();

		m_Logger.LogInfoNL(L"DXShaderLibrary [~()] | Destroyed.");
	}

	void DXShaderLibrary::Init(DXDevice& device) noexcept
	{
		m_Logger.LogFatalNLIf(
			m_Initialized,
			L"DXShaderLibrary [Init] | Initialization has been attempted after initialization already occured."
		);

		CreateShaderSets(device);

		m_Logger.LogInfoNL(L"DXShaderLibrary [Init] | Initialized.");

		m_Initialized = true;
		m_Shutdown = false;
	}

	void DXShaderLibrary::Shutdown() noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"DXShaderLibrary [Shutdown] | Shutdown has been attempted before initialization has occured."
		);

		m_Logger.LogFatalNLIf(
			m_Shutdown,
			L"DXShaderLibrary [Shutdown] | Shutdown has been attempted after shutdown has already occured."
		);

		m_CompiledShaderPath.clear();
		m_ShaderSets.clear();

		m_Logger.LogInfoNL(L"DXShaderLibrary [Shutdown] | Shutdown.");

		m_Initialized = false;
		m_Shutdown = true;
	}

	[[nodiscard]] std::weak_ptr<IDXShaderSet> DXShaderLibrary::GetSetOfType(DXShaderSetType setType) noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"DXShaderLibrary [GetSetOfType] | Attempted to retrieve a shader set before initialization."
		);

		m_Logger.LogFatalNLIf(
			setType == DXShaderSetType::INVALID,
			L"DXShaderLibrary [GetSetOfType] | Attempted to a retrieve a shader set from an invalid setType."
		);

		m_Logger.LogFatalNLIf(m_ShaderSets.empty(), L"DXShaderLibrary [GetSetOfType] | No shader sets are present.");

		for (std::shared_ptr<IDXShaderSet>& pShaderSet : m_ShaderSets)
			if (pShaderSet->Type == setType)
				return std::weak_ptr(pShaderSet);

		m_Logger.LogFatalNL(L"DXShaderLibrary [GetSetOfType] | Failed to retrieve a matching shader set.");
		return std::weak_ptr<IDXShaderSet>();
	}

	void DXShaderLibrary::LoadShaders() noexcept
	{
		m_ShaderSets.reserve(S_EXPECTED_NUM_SHADER_SETS);

		GetCompiledShaderDirectory();

		std::vector<DXShaderData> shaderData;
		GetAllShaderData(shaderData, m_CompiledShaderPath);

		std::wstring message;
		for (DXShaderSetType setType : G_IMPLEMENTED_SET_TYPES)
		{
			const DXShaderData* pVertexData = nullptr;
			const DXShaderData* pPixelData = nullptr;

			for (const DXShaderData& data : shaderData)
			{
				if (data.CorrespondingShaderSet == DXShaderSetType::INVALID)
					m_Logger.LogFatalNLAppend(
						L"DXShaderLibrary [LoadShaders] | Collected shader data has an invalid DXShaderSetType : ", 
						data.Filename
					);
				else if (data.CorrespondingShaderSet != setType)
					continue;

				switch (data.Type)
				{
				case DXShaderType::INVALID:
					m_Logger.LogFatalNLAppend(
						L"DXShaderLibrary [LoadShaders] | Collected shader data has an invalid DXShaderType : ", 
						data.Filename
					);
					return;
				case DXShaderType::VERTEX:
					m_Logger.LogFatalNLAppendIf(
						pVertexData != nullptr,
						L"DXShaderLibrary [LoadShaders] | A second vertex shader was found for the same CMShaderSetType.",
						data.Filename
					);

					pVertexData = &data;
					break;
				case DXShaderType::PIXEL:
					m_Logger.LogFatalNLAppendIf(
						pVertexData != nullptr,
						L"DXShaderLibrary [LoadShaders] | A second pixel shader was found for the same CMShaderSetType.",
						data.Filename
					);

					pPixelData = &data;
					break;
				default:
					m_Logger.LogFatalNL(L"DXShaderLibrary [LoadShaders] | No matching cases found for the CMShaderType of a shader data.");
				}
			}

			m_Logger.LogFatalNLAppendIf(
				pVertexData == nullptr,
				L"DXShaderLibrary [LoadShaders] | No vertex shader was found for the CMShaderSetType : ",
				DXShaderData::ShaderSetTypeToWStrView(setType)
			);

			m_Logger.LogFatalNLAppendIf(
				pPixelData == nullptr,
				L"DXShaderLibrary [LoadShaders] | No pixel shader was found for the CMShaderSetType : ",
				DXShaderData::ShaderSetTypeToWStrView(setType)
			);

			switch (setType)
			{
			case DXShaderSetType::QUAD:
				if (pVertexData != nullptr && pPixelData != nullptr)
					m_ShaderSets.emplace_back(std::make_shared<DXShaderSetQuad>(*pVertexData, *pPixelData));

				break;
			case DXShaderSetType::QUAD_OUTLINED:
				if (pVertexData != nullptr && pPixelData != nullptr)
					m_ShaderSets.emplace_back(std::make_shared<DXShaderSetQuadOutlined>(*pVertexData, *pPixelData));

				break;
			case DXShaderSetType::QUAD_DEPTH:
				if (pVertexData != nullptr && pPixelData != nullptr)
					m_ShaderSets.emplace_back(std::make_shared<DXShaderSetQuadDepth>(*pVertexData, *pPixelData));

				break;
			case DXShaderSetType::CIRCLE:
				if (pVertexData != nullptr && pPixelData != nullptr)
					m_ShaderSets.emplace_back(std::make_shared<DXShaderSetCircle>(*pVertexData, *pPixelData));

				break;
			default:
				m_Logger.LogFatalNL(L"DXShaderLibrary [LoadShaders] | Failed to make a shader set of a DXShaderSetType.");
			}
		}

		m_Logger.LogFatalNLVariadicIf(
			m_ShaderSets.size() != S_EXPECTED_NUM_SHADER_SETS,
			L"DXShaderLibrary [Init] | Failed to collect the expected amount of shader sets.",
			L"(Collected : ", m_ShaderSets.size(),
			L" | Expected : ", S_EXPECTED_NUM_SHADER_SETS
		);
	}

	void DXShaderLibrary::CreateShaderSets(DXDevice& device) noexcept
	{
		for (std::shared_ptr<IDXShaderSet>& shaderSet : m_ShaderSets)
			shaderSet->CreateShaderSet(device, m_Logger);
	}

	void DXShaderLibrary::GetCompiledShaderDirectory() noexcept
	{
		std::filesystem::path workingDirectory = std::filesystem::current_path();

		m_CompiledShaderPath = workingDirectory / "build" / CM_CONFIG / "out" / "CMEngine";

		m_Logger.LogInfoNLAppend(
			L"DXShaderLibrary [GetCompiledShaderDirectory] | Compiled shader path : ",
			m_CompiledShaderPath
		);

		m_Logger.LogFatalNLIf(
			!std::filesystem::exists(m_CompiledShaderPath),
			L"DXShaderLibrary [GetCompiledShaderDirectory] | Shader directory doesn't exist."
		);

		m_Logger.LogFatalNLIf(
			!std::filesystem::is_directory(m_CompiledShaderPath),
			L"DXShaderLibrary [GetCompiledShaderDirectory] | Shader directory wasn't a directory."
		);
	}

	void DXShaderLibrary::GetAllShaderData(std::vector<DXShaderData>& outData, const std::filesystem::path& compiledShaderPath) noexcept
	{
		outData.reserve(S_EXPECTED_NUM_SHADERS);
		size_t totalShadersPresent = TotalCompiledShaders(compiledShaderPath);

		m_Logger.LogFatalNLIf(totalShadersPresent == 0, L"DXShaderLibrary [GetAllShaderData] | No shaders were found.");

		m_Logger.LogFatalNLVariadicIf(
			totalShadersPresent != S_EXPECTED_NUM_SHADERS,
			L"DXShaderLibrary [GetAllShaderData] | The amount of compiled shaders didn't match the expected amount.",
			L"(Collected : ", totalShadersPresent,
			L" | Expected : ", S_EXPECTED_NUM_SHADERS
		);

		Microsoft::WRL::ComPtr<ID3DBlob> pBlob;
		for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(compiledShaderPath))
		{
			std::filesystem::path entryPath = entry.path();
			std::wstring entryWStr = entryPath.wstring();

			if (entryPath.extension() != ".cso")
				continue;

			std::wstring fileName = entryPath.filename();

			// (Number of characters before the flag; e.g., .....PS.cso)
			m_Logger.LogFatalNLAppendIf(
				fileName.size() - 6 <= 0,
				L"DXShaderLibrary [GetAllShaderData] | Invalid shader : ",
				fileName
			);

			// Extract the flag (e.g., "VS", "PS") of the shader.
			std::wstring shaderFlag = fileName.substr(fileName.size() - 6, 2);
			DXShaderType shaderType = DXShaderType::INVALID;

			if (shaderFlag == DXShaderData::S_VERTEX_FLAG)
				shaderType = DXShaderType::VERTEX;
			else if (shaderFlag == DXShaderData::S_PIXEL_FLAG)
				shaderType = DXShaderType::PIXEL;
			else
				m_Logger.LogFatalNLVariadic(
					-1,
					L"DXShaderLibrary [GetAllShaderData] | Invalid shader : ",
					fileName,
					". Expected flag in shader name. (\"VS\", \"PS\")"
				);

			m_Logger.LogFatalNLAppendIf(
				FAILED(D3DReadFileToBlob(entryWStr.data(), &pBlob)),
				L"DXShaderLibrary [GetAllShaderData] | Failed to read compiled shader : ",
				fileName
			);

			DXShaderSetType setType = CorrespondingSetType(fileName);

			m_Logger.LogFatalNLAppendIf(
				setType == DXShaderSetType::INVALID,
				L"DXShaderLibrary [GetAllShaderData] | Failed to find a matching shader set type for the shader : ",
				fileName
			);

			m_Logger.LogInfoNLAppend(
				L"DXShaderLibrary [GetAllShaderData] | Found valid shader : ",
				fileName
			);

			m_Logger.LogInfoNLAppend(
				L"DXShaderLibrary [GetAllShaderData] | Shader flag : ",
				shaderFlag
			);

			m_Logger.LogInfoNLAppend(
				L"DXShaderLibrary [GetAllShaderData] | Shader set type : ",
				DXShaderData::ShaderSetTypeToWStrView(setType)
			);

			outData.emplace_back(setType, shaderType, pBlob, fileName);
		}

		m_Logger.LogInfoNLAppend(
			L"DXShaderLibrary [GetAllShaderData] | Total shaders collected : ",
			outData.size()
		);

		m_Logger.LogFatalNLVariadicIf(
			outData.size() != S_EXPECTED_NUM_SHADERS,
			L"DXShaderLibrary [GetAllShaderData] | Failed to collect the expected amount of shaders : ",
			L"(Collected : ", outData.size(),
			L" | Expected : ", S_EXPECTED_NUM_SHADERS, L')'
		);
	}

	[[nodiscard]] size_t DXShaderLibrary::TotalCompiledShaders(const std::filesystem::path& compiledShaderPath) const noexcept
	{
		size_t totalShaders = 0;

		for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(compiledShaderPath))
		{
			const std::filesystem::path& path = entry.path();

			if (path.extension() == L".cso")
				++totalShaders;
		}

		return totalShaders;
	}

	[[nodiscard]] DXShaderSetType DXShaderLibrary::CorrespondingSetType(const std::wstring& fileName) const noexcept
	{
		if (fileName == S_CMQUAD_VS_NAME || fileName == S_CMQUAD_PS_NAME)
			return DXShaderSetType::QUAD;
		else if (fileName == S_CMQUAD_OUTLINED_VS_NAME || fileName == S_CMQUAD_OUTLINED_PS_NAME)
			return DXShaderSetType::QUAD_OUTLINED;
		else if (fileName == S_CMQUAD_DEPTH_VS_NAME || fileName == S_CMQUAD_DEPTH_PS_NAME)
			return DXShaderSetType::QUAD_DEPTH;
		else if (fileName == S_CMCIRCLE_VS_NAME || fileName == S_CMCIRCLE_PS_NAME)
			return DXShaderSetType::CIRCLE;

		return DXShaderSetType::INVALID;
	}
}