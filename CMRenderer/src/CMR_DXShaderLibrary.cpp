#include "CMR_PCH.hpp"
#include "CMR_DXShaderLibrary.hpp"
#include "CMC_Paths.hpp"

namespace CMRenderer::CMDirectX
{
	DXShaderLibrary::DXShaderLibrary(CMCommon::CMLoggerWide& cmLoggerRef) noexcept
		: m_CMLoggerRef(cmLoggerRef)
	{
		LoadShaders();

		m_CMLoggerRef.LogInfoNL(L"DXShaderLibrary [()] | Constructed.");
	}

	DXShaderLibrary::~DXShaderLibrary() noexcept
	{
		if (m_Initialized)
			Shutdown();

		m_CMLoggerRef.LogInfoNL(L"DXShaderLibrary [~()] | Destroyed.");
	}

	void DXShaderLibrary::Init(Components::DXDevice& deviceRef) noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(m_Initialized, L"DXShaderLibrary [Init] | Initialization has been attempted after initialization already occured.");

		CreateShaderSets(deviceRef);

		m_Initialized = true;
		m_Shutdown = false;
		m_CMLoggerRef.LogInfoNL(L"DXShaderLibrary [Init] | Initialized.");
	}

	void DXShaderLibrary::Shutdown() noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(!m_Initialized, L"DXShaderLibrary [Shutdown] | Shutdown has been attempted before initialization has occured.");
		m_CMLoggerRef.LogFatalNLIf(m_Shutdown, L"DXShaderLibrary [Shutdown] | Shutdown has been attempted after shutdown has already occured.");

		m_CompiledShaderDirectory.clear();
		m_ShaderSets.clear();

		m_Initialized = false;
		m_Shutdown = true;
		m_CMLoggerRef.LogInfoNL(L"DXShaderLibrary [Shutdown] | Shutdown.");
	}

	[[nodiscard]] std::weak_ptr<IDXShaderSet> DXShaderLibrary::GetSetOfType(DXShaderSetType setType) noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(!m_Initialized, L"DXShaderLibrary [GetSetOfType] | Attempted to retrieve a shader set before initialization.");

		m_CMLoggerRef.LogFatalNLIf(
			setType == DXShaderSetType::INVALID,
			L"DXShaderLibrary [GetSetOfType] | Attempted to a retrieve a shader set from an invalid setType."
		);

		m_CMLoggerRef.LogFatalNLIf(m_ShaderSets.size() == 0, L"DXShaderLibrary [GetSetOfType] | No shader sets are present.");

		for (std::shared_ptr<IDXShaderSet>& pShaderSet : m_ShaderSets)
			if (pShaderSet->Type == setType)
				return std::weak_ptr(pShaderSet);

		m_CMLoggerRef.LogFatalNL(L"DXShaderLibrary [GetSetOfType] | Failed to retrieve a matching shader set.");
		return std::weak_ptr<IDXShaderSet>();
	}

	void DXShaderLibrary::LoadShaders() noexcept
	{
		m_ShaderSets.reserve(S_EXPECTED_NUM_SHADER_SETS);

		std::filesystem::path compiledShaderPath;
		GetCompiledShaderDirectory(compiledShaderPath);

		std::vector<DXShaderData> shaderData;
		GetAllShaderData(shaderData, compiledShaderPath);

		std::wstring message;
		for (DXShaderSetType setType : G_IMPLEMENTED_SET_TYPES)
		{
			const DXShaderData* pVertexData = nullptr;
			const DXShaderData* pPixelData = nullptr;

			for (const DXShaderData& data : shaderData)
			{
				if (data.CorrespondingShaderSet == DXShaderSetType::INVALID)
					m_CMLoggerRef.LogFatalNLAppend(L"DXShaderLibrary [LoadShaders] | Collected shader data has an invalid DXShaderSetType : ", data.Filename);
				else if (data.CorrespondingShaderSet != setType)
					continue;

				switch (data.Type)
				{
				case DXShaderType::INVALID:
					m_CMLoggerRef.LogFatalNLAppend(L"DXShaderLibrary [LoadShaders] | Collected shader data has an invalid DXShaderType : ", data.Filename);
					return;
				case DXShaderType::VERTEX:
					m_CMLoggerRef.LogFatalNLAppendIf(
						pVertexData != nullptr,
						L"DXShaderLibrary [LoadShaders] | A second vertex shader was found for the same CMShaderSetType.",
						data.Filename
					);

					pVertexData = &data;
					break;
				case DXShaderType::PIXEL:
					m_CMLoggerRef.LogFatalNLAppendIf(
						pVertexData != nullptr,
						L"DXShaderLibrary [LoadShaders] | A second pixel shader was found for the same CMShaderSetType.",
						data.Filename
					);

					pPixelData = &data;
					break;
				default:
					m_CMLoggerRef.LogFatalNL(L"DXShaderLibrary [LoadShaders] | No matching cases found for the CMShaderType of a shader data.");
				}
			}

			m_CMLoggerRef.LogFatalNLAppendIf(
				pVertexData == nullptr,
				L"DXShaderLibrary [LoadShaders] | No vertex shader was found for the CMShaderSetType : ",
				DXShaderData::ShaderSetTypeToWStrView(setType)
			);

			m_CMLoggerRef.LogFatalNLAppendIf(
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
			case DXShaderSetType::QUAD_DEPTH:
				if (pVertexData != nullptr && pPixelData != nullptr)
					m_ShaderSets.emplace_back(std::make_shared<DXShaderSetQuadDepth>(*pVertexData, *pPixelData));

				break;
			case DXShaderSetType::CIRCLE:
				if (pVertexData != nullptr && pPixelData != nullptr)
					m_ShaderSets.emplace_back(std::make_shared<DXShaderSetCircle>(*pVertexData, *pPixelData));

				break;
			default:
				m_CMLoggerRef.LogFatalNL(L"DXShaderLibrary [LoadShaders] | Failed to make a shader set of a DXShaderSetType.");
			}
		}

		m_CMLoggerRef.LogFatalNLVariadicIf(
			m_ShaderSets.size() != S_EXPECTED_NUM_SHADER_SETS,
			L"DXShaderLibrary [Init] | Failed to collect the expected amount of shader sets.",
			L"(Collected : ", m_ShaderSets.size(),
			L" | Expected : ", S_EXPECTED_NUM_SHADER_SETS
		);
	}

	void DXShaderLibrary::CreateShaderSets(Components::DXDevice& deviceRef) noexcept
	{
		for (std::shared_ptr<IDXShaderSet>& shaderSet : m_ShaderSets)
			shaderSet->CreateShaderSet(deviceRef, m_CMLoggerRef);
	}

	void DXShaderLibrary::GetCompiledShaderDirectory(std::filesystem::path& outPathRef) noexcept
	{
		outPathRef = Utility::RendererOutDirectoryRelativeToEngine();
		m_CompiledShaderDirectory = outPathRef.wstring();

		m_CMLoggerRef.LogInfoNLAppend(
			L"DXShaderLibrary [GetCompiledShaderDirectory] | Compiled shader path : ",
			m_CompiledShaderDirectory
		);

		m_CMLoggerRef.LogFatalNLIf(!std::filesystem::exists(outPathRef), L"DXShaderLibrary [GetCompiledShaderDirectory] | Shader directory doesn't exist.");
		m_CMLoggerRef.LogFatalNLIf(!std::filesystem::is_directory(outPathRef), L"DXShaderLibrary [GetCompiledShaderDirectory] | Shader directory wasn't a directory.");
	}

	void DXShaderLibrary::GetAllShaderData(std::vector<DXShaderData>& outDataRef, const std::filesystem::path& compiledShaderPathRef) noexcept
	{
		outDataRef.reserve(S_EXPECTED_NUM_SHADERS);
		size_t totalShadersPresent = TotalCompiledShaders(compiledShaderPathRef);

		m_CMLoggerRef.LogFatalNLIf(totalShadersPresent == 0, L"DXShaderLibrary [GetAllShaderData] | No shaders were found.");

		m_CMLoggerRef.LogFatalNLVariadicIf(
			totalShadersPresent != S_EXPECTED_NUM_SHADERS,
			L"DXShaderLibrary [GetAllShaderData] | The amount of compiled shaders didn't match the expected amount.",
			L"(Collected : ", totalShadersPresent,
			L" | Expected : ", S_EXPECTED_NUM_SHADERS
		);

		Microsoft::WRL::ComPtr<ID3DBlob> pBlob;
		for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(compiledShaderPathRef))
		{
			std::filesystem::path entryPath = entry.path();
			std::wstring entryWStr = entryPath.wstring();

			if (entryPath.extension() != ".cso")
				continue;

			std::wstring fileName = entryPath.filename();

			// (Number of characters before the flag; e.g., .....PS.cso)
			m_CMLoggerRef.LogFatalNLAppendIf(
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
				m_CMLoggerRef.LogFatalNLVariadic(
					-1,
					L"DXShaderLibrary [GetAllShaderData] | Invalid shader : ",
					fileName,
					". Expected flag in shader name. (\"VS\", \"PS\")"
				);

			m_CMLoggerRef.LogFatalNLAppendIf(
				FAILED(D3DReadFileToBlob(entryWStr.data(), &pBlob)),
				L"DXShaderLibrary [GetAllShaderData] | Failed to read compiled shader : ",
				fileName
			);

			DXShaderSetType setType = CorrespondingSetType(fileName);

			m_CMLoggerRef.LogFatalNLAppendIf(
				setType == DXShaderSetType::INVALID,
				L"DXShaderLibrary [GetAllShaderData] | Failed to find a matching shader set type for the shader : ",
				fileName
			);

			m_CMLoggerRef.LogInfoNLAppend(
				L"DXShaderLibrary [GetAllShaderData] | Found valid shader : ",
				fileName
			);

			m_CMLoggerRef.LogInfoNLAppend(
				L"DXShaderLibrary [GetAllShaderData] | Shader flag : ",
				shaderFlag
			);

			m_CMLoggerRef.LogInfoNLAppend(
				L"DXShaderLibrary [GetAllShaderData] | Shader set type : ",
				DXShaderData::ShaderSetTypeToWStrView(setType)
			);

			outDataRef.emplace_back(setType, shaderType, pBlob, fileName);
		}

		m_CMLoggerRef.LogInfoNLAppend(
			L"DXShaderLibrary [GetAllShaderData] | Total shaders collected : ",
			outDataRef.size()
		);

		m_CMLoggerRef.LogFatalNLVariadicIf(
			outDataRef.size() != S_EXPECTED_NUM_SHADERS,
			L"DXShaderLibrary [GetAllShaderData] | Failed to collect the expected amount of shaders : ",
			L"(Collected : ", outDataRef.size(),
			L" | Expected : ", S_EXPECTED_NUM_SHADERS, L')'
		);
	}

	[[nodiscard]] size_t DXShaderLibrary::TotalCompiledShaders(const std::filesystem::path& compiledShaderPathRef) const noexcept
	{
		size_t totalShaders = 0;

		for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(compiledShaderPathRef))
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
		else if (fileName == S_CMQUAD_DEPTH_VS_NAME || fileName == S_CMQUAD_DEPTH_PS_NAME)
			return DXShaderSetType::QUAD_DEPTH;
		else if (fileName == S_CMCIRCLE_VS_NAME || fileName == S_CMCIRCLE_PS_NAME)
			return DXShaderSetType::CIRCLE;

		return DXShaderSetType::INVALID;
	}
}