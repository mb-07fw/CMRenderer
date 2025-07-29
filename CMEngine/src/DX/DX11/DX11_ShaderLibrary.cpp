#include "Core/PCH.hpp"
#include "Core/Paths.hpp"
#include "DX/DX11/DX11_ShaderLibrary.hpp"

namespace CMEngine::DX::DX11
{
	ShaderLibrary::ShaderLibrary(Common::LoggerWide& logger) noexcept
		: m_Logger(logger)
	{
		LoadShaders();

		m_Logger.LogInfoNL(L"ShaderLibrary [()] | Constructed.");
	}

	ShaderLibrary::~ShaderLibrary() noexcept
	{
		if (m_Initialized)
			Shutdown();

		m_Logger.LogInfoNL(L"ShaderLibrary [~()] | Destroyed.");
	}

	void ShaderLibrary::Init(Device& device) noexcept
	{
		m_Logger.LogFatalNLIf(
			m_Initialized,
			L"ShaderLibrary [Init] | Initialization has been attempted after initialization already occured."
		);

		CreateShaderSets(device);

		m_Logger.LogInfoNL(L"ShaderLibrary [Init] | Initialized.");

		m_Initialized = true;
		m_Shutdown = false;
	}

	void ShaderLibrary::Shutdown() noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"ShaderLibrary [Shutdown] | Shutdown has been attempted before initialization has occured."
		);

		m_Logger.LogFatalNLIf(
			m_Shutdown,
			L"ShaderLibrary [Shutdown] | Shutdown has been attempted after shutdown has already occured."
		);

		m_CompiledShaderPath.clear();
		m_ShaderSets.clear();

		m_Logger.LogInfoNL(L"ShaderLibrary [Shutdown] | Shutdown.");

		m_Initialized = false;
		m_Shutdown = true;
	}

	[[nodiscard]] std::weak_ptr<IShaderSet> ShaderLibrary::GetSetOfType(ShaderSetType setType) noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"ShaderLibrary [GetSetOfType] | Attempted to retrieve a shader set before initialization."
		);

		m_Logger.LogFatalNLIf(
			setType == ShaderSetType::INVALID,
			L"ShaderLibrary [GetSetOfType] | Attempted to a retrieve a shader set from an invalid setType."
		);

		m_Logger.LogFatalNLIf(m_ShaderSets.empty(), L"ShaderLibrary [GetSetOfType] | No shader sets are present.");

		for (std::shared_ptr<IShaderSet>& pShaderSet : m_ShaderSets)
			if (pShaderSet->Type == setType)
				return std::weak_ptr(pShaderSet);

		m_Logger.LogFatalNL(L"ShaderLibrary [GetSetOfType] | Failed to retrieve a matching shader set.");
		return std::weak_ptr<IShaderSet>();
	}

	void ShaderLibrary::LoadShaders() noexcept
	{
		m_ShaderSets.reserve(S_EXPECTED_NUM_SHADER_SETS);

		GetCompiledShaderDirectory();

		std::vector<ShaderData> shaderData;
		GetAllShaderData(shaderData, m_CompiledShaderPath);

		std::wstring message;
		for (ShaderSetType setType : G_IMPLEMENTED_SET_TYPES)
		{
			const ShaderData* pVertexData = nullptr;
			const ShaderData* pPixelData = nullptr;

			for (const ShaderData& data : shaderData)
			{
				if (data.CorrespondingShaderSet == ShaderSetType::INVALID)
					m_Logger.LogFatalNLAppend(
						L"ShaderLibrary [LoadShaders] | Collected shader data has an invalid ShaderSetType : ", 
						data.Filename
					);
				else if (data.CorrespondingShaderSet != setType)
					continue;

				switch (data.Type)
				{
				case ShaderType::INVALID:
					m_Logger.LogFatalNLAppend(
						L"ShaderLibrary [LoadShaders] | Collected shader data has an invalid ShaderType : ", 
						data.Filename
					);
					return;
				case ShaderType::VERTEX:
					m_Logger.LogFatalNLAppendIf(
						pVertexData != nullptr,
						L"ShaderLibrary [LoadShaders] | A second vertex shader was found for the same ShaderSetType.",
						data.Filename
					);

					pVertexData = &data;
					break;
				case ShaderType::PIXEL:
					m_Logger.LogFatalNLAppendIf(
						pVertexData != nullptr,
						L"ShaderLibrary [LoadShaders] | A second pixel shader was found for the same ShaderSetType.",
						data.Filename
					);

					pPixelData = &data;
					break;
				default:
					m_Logger.LogFatalNL(L"ShaderLibrary [LoadShaders] | No matching cases found for the ShaderType of a shader data.");
				}
			}

			m_Logger.LogFatalNLAppendIf(
				pVertexData == nullptr,
				L"ShaderLibrary [LoadShaders] | No vertex shader was found for the ShaderSetType : ",
				ShaderData::ShaderSetTypeToWStrView(setType)
			);

			m_Logger.LogFatalNLAppendIf(
				pPixelData == nullptr,
				L"ShaderLibrary [LoadShaders] | No pixel shader was found for the ShaderSetType : ",
				ShaderData::ShaderSetTypeToWStrView(setType)
			);

			switch (setType)
			{
			case ShaderSetType::QUAD:
				if (pVertexData != nullptr && pPixelData != nullptr)
					m_ShaderSets.emplace_back(std::make_shared<ShaderSetQuad>(*pVertexData, *pPixelData));

				break;
			case ShaderSetType::QUAD_OUTLINED:
				if (pVertexData != nullptr && pPixelData != nullptr)
					m_ShaderSets.emplace_back(std::make_shared<ShaderSetQuadOutlined>(*pVertexData, *pPixelData));

				break;
			case ShaderSetType::QUAD_DEPTH:
				if (pVertexData != nullptr && pPixelData != nullptr)
					m_ShaderSets.emplace_back(std::make_shared<ShaderSetQuadDepth>(*pVertexData, *pPixelData));

				break;
			case ShaderSetType::CIRCLE:
				if (pVertexData != nullptr && pPixelData != nullptr)
					m_ShaderSets.emplace_back(std::make_shared<ShaderSetCircle>(*pVertexData, *pPixelData));

				break;
			default:
				m_Logger.LogFatalNL(L"ShaderLibrary [LoadShaders] | Failed to make a shader set of a ShaderSetType.");
			}
		}

		m_Logger.LogFatalNLVariadicIf(
			m_ShaderSets.size() != S_EXPECTED_NUM_SHADER_SETS,
			L"ShaderLibrary [Init] | Failed to collect the expected amount of shader sets.",
			L"(Collected : ", m_ShaderSets.size(),
			L" | Expected : ", S_EXPECTED_NUM_SHADER_SETS
		);
	}

	void ShaderLibrary::CreateShaderSets(Device& device) noexcept
	{
		for (std::shared_ptr<IShaderSet>& shaderSet : m_ShaderSets)
			shaderSet->CreateShaderSet(device, m_Logger);
	}

	void ShaderLibrary::GetCompiledShaderDirectory() noexcept
	{
		m_CompiledShaderPath = Core::CMEOutDirectory();

		m_Logger.LogInfoNLAppend(
			L"ShaderLibrary [GetCompiledShaderDirectory] | Compiled shader path : ",
			m_CompiledShaderPath
		);

		m_Logger.LogFatalNLIf(
			!std::filesystem::exists(m_CompiledShaderPath),
			L"ShaderLibrary [GetCompiledShaderDirectory] | Shader directory doesn't exist."
		);

		m_Logger.LogFatalNLIf(
			!std::filesystem::is_directory(m_CompiledShaderPath),
			L"ShaderLibrary [GetCompiledShaderDirectory] | Shader directory wasn't a directory."
		);
	}

	void ShaderLibrary::GetAllShaderData(std::vector<ShaderData>& outData, const std::filesystem::path& compiledShaderPath) noexcept
	{
		outData.reserve(S_EXPECTED_NUM_SHADERS);
		size_t totalShadersPresent = TotalCompiledShaders(compiledShaderPath);

		constexpr std::wstring_view FuncTag = L"ShaderLibrary [GetAllShaderData] | ";
		std::wstring compiledShaderPathWStr = compiledShaderPath.wstring();

		m_Logger.LogFatalNLFormattedIf(
			totalShadersPresent == 0,
			FuncTag,
			L"No shaders were found in the expected shader output directory : `{}`",
			compiledShaderPathWStr
		);

		m_Logger.LogFatalNLFormattedIf(
			totalShadersPresent != S_EXPECTED_NUM_SHADERS,
			FuncTag,
			L"The amount of compiled shaders didn't match the expected amount. ",
			L"(Collected : `{}` | Expected : `{}`)",
			totalShadersPresent,
			S_EXPECTED_NUM_SHADERS
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
			m_Logger.LogFatalNLFormattedIf(
				fileName.size() - 6 <= 0,
				FuncTag,
				L"Invalid shader : `{}`",
				fileName
			);

			// Extract the flag (e.g., "VS", "PS") of the shader.
			std::wstring shaderFlag = fileName.substr(fileName.size() - 6, 2);
			ShaderType shaderType = ShaderType::INVALID;

			if (shaderFlag == ShaderData::S_VERTEX_FLAG)
				shaderType = ShaderType::VERTEX;
			else if (shaderFlag == ShaderData::S_PIXEL_FLAG)
				shaderType = ShaderType::PIXEL;
			else
				m_Logger.LogFatalNLFormatted(
					FuncTag,
					L"Invalid shader : `{}`. Expected flag in shader name. (`VS`, `PS`)",
					fileName
				);

			m_Logger.LogFatalNLFormattedIf(
				FAILED(D3DReadFileToBlob(entryWStr.data(), &pBlob)),
				FuncTag,
				L"Failed to read compiled shader : `{}`.",
				fileName
			);

			ShaderSetType setType = CorrespondingSetType(fileName);

			m_Logger.LogFatalNLFormattedIf(
				setType == ShaderSetType::INVALID,
				FuncTag,
				L"Failed to find a matching shader set type for the shader : `{}`.",
				fileName
			);

			m_Logger.LogInfoNLFormatted(
				FuncTag,
				L"Found valid shader : `{}`.",
				fileName
			);

			m_Logger.LogInfoNLFormatted(
				FuncTag,
				L"Shader flag : `{}`.",
				shaderFlag
			);

			const wchar_t* pSetTypeWStr = ShaderData::ShaderSetTypeToWStrView(setType).data();

			m_Logger.LogInfoNLFormatted(
				FuncTag,
				L"Shader set type : `{}`.",
				pSetTypeWStr
			);

			outData.emplace_back(setType, shaderType, pBlob, fileName);
		}

		size_t numShaderData = outData.size();

		m_Logger.LogInfoNLFormatted(
			FuncTag,
			L"Total shaders collected : `{}`.",
			numShaderData
		);

		m_Logger.LogFatalNLFormattedIf(
			outData.size() != S_EXPECTED_NUM_SHADERS,
			FuncTag,
			L"Failed to collect the expected amount of shaders. "
			L"(Collected : `{}` | Expected : `{}`).",
			numShaderData,
			S_EXPECTED_NUM_SHADERS
		);
	}

	[[nodiscard]] size_t ShaderLibrary::TotalCompiledShaders(const std::filesystem::path& compiledShaderPath) const noexcept
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

	[[nodiscard]] ShaderSetType ShaderLibrary::CorrespondingSetType(const std::wstring& fileName) const noexcept
	{
		if (fileName == S_QUAD_VS_NAME || fileName == S_QUAD_PS_NAME)
			return ShaderSetType::QUAD;
		else if (fileName == S_QUAD_OUTLINED_VS_NAME || fileName == S_QUAD_OUTLINED_PS_NAME)
			return ShaderSetType::QUAD_OUTLINED;
		else if (fileName == S_QUAD_DEPTH_VS_NAME || fileName == S_QUAD_DEPTH_PS_NAME)
			return ShaderSetType::QUAD_DEPTH;
		else if (fileName == S_CIRCLE_VS_NAME || fileName == S_CIRCLE_PS_NAME)
			return ShaderSetType::CIRCLE;

		return ShaderSetType::INVALID;
	}
}