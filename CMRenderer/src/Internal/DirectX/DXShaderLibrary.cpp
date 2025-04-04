#include "Core/CMPCH.hpp"
#include "Internal/DirectX/DXShaderLibrary.hpp"

namespace CMRenderer::CMDirectX
{
	CMShaderLibrary::CMShaderLibrary(Utility::CMLoggerWide& cmLoggerRef) noexcept
		: m_CMLoggerRef(cmLoggerRef)
	{
		m_ShaderSets.reserve(S_EXPECTED_NUM_SHADER_SETS);
		m_CMLoggerRef.LogInfoNL(L"CMShaderLibrary [()] | Constructed.");
	}

	CMShaderLibrary::~CMShaderLibrary() noexcept
	{
		if (m_Initialized)
			Shutdown();

		m_CMLoggerRef.LogInfoNL(L"CMShaderLibrary [~()] | Destroyed.");
	}

	void CMShaderLibrary::Init() noexcept
	{
		if (m_Initialized)
		{
			m_CMLoggerRef.LogWarningNL(L"CMShaderLibrary [Init] | Initialization has been attempted after initialization already occured.");
			return;
		}

		std::filesystem::path compiledShaderPath;
		GetCompiledShaderDirectory(compiledShaderPath);

		std::vector<CMShaderData> shaderData;
		GetAllShaderData(shaderData, compiledShaderPath);

		std::wstring message;
		for (CMImplementedShaderType implementedType : G_IMPLEMENTED_SHADER_TYPES)
		{
			const CMShaderData* pVertexData = nullptr;
			const CMShaderData* pPixelData = nullptr;

			for (const CMShaderData& data : shaderData)
			{
				if (data.ImplementedType != implementedType)
					continue;

				switch (data.Type)
				{
				case CMShaderType::INVALID:
					m_CMLoggerRef.LogFatalNL(L"CMShaderLibrary [Init] | A collected shader has an invalid CMShaderType.");
					break; // LogFatal will terminate the program anyway...
				case CMShaderType::VERTEX:
					if (pVertexData != nullptr)
						m_CMLoggerRef.LogFatalNL(L"CMShaderLibrary [Init] | A second vertex shader was found for the same CMImplementedShaderType.");

					pVertexData = &data;
					break;
				case CMShaderType::PIXEL:
					if (pVertexData != nullptr)
						m_CMLoggerRef.LogFatalNL(L"CMShaderLibrary [Init] | A second pixel shader was found for the same CMImplementedShaderType.");

					pPixelData = &data;
					break;
				default:
					m_CMLoggerRef.LogFatalNL(L"CMShaderLibrary [Init] | No matching cases found for the CMShaderType of a shader.");
				}
			}

			if (pVertexData == nullptr)
			{
				m_CMLoggerRef.LogFatalNLAppend(
					L"CMShaderLibrary [Init] | No vertex shader was found for the CMImplementedType : ",
					CMShaderData::ImplementedToWStrView(implementedType)
				);
				return; // Here so Intelisense doesn't yell at me for dereferencing a null pointer.
			}
			else if (pPixelData == nullptr)
			{
				m_CMLoggerRef.LogFatalNLAppend(
					L"CMShaderLibrary [Init] | No pixel shader was found for the CMImplementedType : ",
					CMShaderData::ImplementedToWStrView(implementedType)
				);
				return; // Here so Intelisense doesn't yell at me for dereferencing a null pointer.
			}

			Utility::CMStaticArray<D3D11_INPUT_ELEMENT_DESC> desc;
			GetCorrespondingDescription(desc, implementedType);

			m_ShaderSets.emplace_back(*pVertexData, *pPixelData, desc, implementedType);
		}

		if (m_ShaderSets.size() != S_EXPECTED_NUM_SHADER_SETS)
		{
			m_CMLoggerRef.LogFatalNLVariadic(
				-1,
				L"CMShaderLibrary [Init] | Failed to collect the expected amount of shader sets.",
				"(Collected : ", m_ShaderSets.size(),
				L" | Expected : ", S_EXPECTED_NUM_SHADER_SETS
			);
		}

		m_Initialized = true;
		m_Shutdown = false;
		m_CMLoggerRef.LogInfoNL(L"CMShaderLibrary [Init] | Initialied.");
	}

	void CMShaderLibrary::Shutdown() noexcept
	{
		if (!m_Initialized)
		{
			m_CMLoggerRef.LogWarningNL(L"CMShaderLibrary [Shutdown] | Shutdown has been attempted before initialization has occured.");
			return;
		}
		else if (m_Shutdown)
		{
			m_CMLoggerRef.LogWarningNL(L"CMShaderLibrary [Shutdown] | Shutdown has been attempted after shutdown has already occured.");
			return;
		}

		m_CompiledShaderDirectory.clear();
		m_ShaderSets.clear();

		m_Initialized = false;
		m_Shutdown = true;
		m_CMLoggerRef.LogInfoNL(L"CMShaderLibrary [Shutdown] | Shutdown.");
	}

	[[nodiscard]] const CMShaderSet& CMShaderLibrary::GetSetOfType(CMImplementedShaderType implementedType) noexcept
	{
		if (!m_Initialized)
			m_CMLoggerRef.LogFatalNL(L"CMShaderLibrary [GetSetOfType] | Attempted to retrieve a shader set before initialization occured.");

		for (const CMShaderSet& set : m_ShaderSets)
			if (set.ImplementedType() == implementedType)
				return set;

		m_CMLoggerRef.LogFatalNL(L"CMShaderLibrary [GetSetOfType] | Couldn't find a shader set with the matching implemented type.");
		return m_ShaderSets.back();
	}

	void CMShaderLibrary::GetCompiledShaderDirectory(std::filesystem::path& outPathRef) noexcept
	{
		std::filesystem::path workingPath = std::filesystem::current_path();

		m_CMLoggerRef.LogInfoNLAppend(
			L"CMShaderLibrary [GetCompiledShaderDirectory] | Working directory : ",
			workingPath.wstring()
		);
	
		outPathRef = workingPath.parent_path() / "build" / CM_CONFIG / "Out";
		m_CompiledShaderDirectory = outPathRef.wstring();

		m_CMLoggerRef.LogInfoNLAppend(
			L"CMShaderLibrary [GetCompiledShaderDirectory] | Compiled shader path : ",
			m_CompiledShaderDirectory
		);

		if (!std::filesystem::exists(outPathRef))
			m_CMLoggerRef.LogFatalNL(L"CMShaderLibrary [GetCompiledShaderDirectory] | Shader directory doesn't exist.");
		else if (!std::filesystem::is_directory(outPathRef))
			m_CMLoggerRef.LogFatalNL(L"CMShaderLibrary [GetCompiledShaderDirectory] | Shader directory wasn't a directory.");
	}

	void CMShaderLibrary::GetAllShaderData(std::vector<CMShaderData>& outDataRef, const std::filesystem::path& compiledShaderPathRef) noexcept
	{
		size_t totalShaders = TotalCompiledShaders(compiledShaderPathRef);
		outDataRef.reserve(totalShaders);

		std::wstring message;

		if (totalShaders == 0)
			m_CMLoggerRef.LogFatalNL(L"CMShaderLibrary [GetAllShaderData] | No shaders were found.");
		else if (totalShaders != S_EXPECTED_NUM_SHADERS)
			m_CMLoggerRef.LogFatalNLVariadic(
				-1,
				L"CMShaderLibrary [GetAllShaderData] | The amount of compiled shaders didn't match the expected amount.",
				"(Collected : ", totalShaders,
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
			if (fileName.size() - 6 <= 0)
			{
				m_CMLoggerRef.LogWarningNLAppend(
					L"CMShaderLibrary [GetAllShaderData] | Invalid shader : ",
					fileName
				);

				continue;
			}

			// Extract the flag (e.g., "VS", "PS") of the shader.
			std::wstring shaderFlag = fileName.substr(fileName.size() - 6, 2);
			CMShaderType shaderType = CMShaderType::INVALID;

			if (shaderFlag == CMShaderData::S_VERTEX_FLAG)
				shaderType = CMShaderType::VERTEX;
			else if (shaderFlag == CMShaderData::S_PIXEL_FLAG)
				shaderType = CMShaderType::PIXEL;
			else
				m_CMLoggerRef.LogFatalNLVariadic(
					-1,
					L"CMShaderLibrary [GetAllShaderData] | Invalid shader : ",
					fileName, ". Expected flag in shader name. (\"VS\", \"PS\")"
				);

			HRESULT hResult = D3DReadFileToBlob(entryWStr.data(), &pBlob);

			if (hResult != S_OK)
				m_CMLoggerRef.LogFatalNLAppend(
					L"CMShaderLibrary [GetAllShaderData] | Failed to read in compiled shader : ",
					fileName
				);

			CMImplementedShaderType implementedType = CorrespondingImplementedType(fileName);
			
			m_CMLoggerRef.LogInfoNLAppend(
				L"CMShaderLibrary [GetAllShaderData] | Found valid shader : ",
				fileName
			);

			m_CMLoggerRef.LogInfoNLAppend(
				L"CMShaderLibrary [GetAllShaderData] | Shader flag : ",
				shaderFlag
			);

			m_CMLoggerRef.LogInfoNLAppend(
				L"CMShaderLibrary [GetAllShaderData] | Implemented type : ",
				CMShaderData::ImplementedToWStrView(implementedType)
			);

			if (implementedType == CMImplementedShaderType::INVALID)
				m_CMLoggerRef.LogFatalNLAppend(
					L"CMShaderLibrary [GetAllShaderData] | Failed to find a matching implemented type for the shader : ",
					fileName
				);
			
			outDataRef.emplace_back(implementedType, shaderType, pBlob, fileName);
		}

		m_CMLoggerRef.LogInfoNLAppend(
			L"CMShaderLibrary [GetAllShaderData] | Total shaders collected : ", 
			outDataRef.size()
		);

		if (outDataRef.size() != S_EXPECTED_NUM_SHADERS)
			m_CMLoggerRef.LogFatalNLVariadic(
				-1,
				L"CMShaderLibrary [GetAllShaderData] | Failed to collect the expected amount of shaders.",
				"(Collected : ", outDataRef.size(),
				L" | Expected : ", S_EXPECTED_NUM_SHADERS
			);
	}

	[[nodiscard]] size_t CMShaderLibrary::TotalCompiledShaders(const std::filesystem::path& compiledShaderPathRef) const noexcept
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

	[[nodiscard]] CMImplementedShaderType CMShaderLibrary::CorrespondingImplementedType(const std::wstring& fileName) const noexcept
	{
		if (fileName == S_DEFAULT_VS_NAME || fileName == S_DEFAULT_PS_NAME)
			return CMImplementedShaderType::DEFAULT;
		else if (fileName == S_DEFAULT3D_VS_NAME || fileName == S_DEFAULT3D_PS_NAME)
			return CMImplementedShaderType::DEFAULT3D;
		else if (fileName == S_POS2D_INTERCOLOR_VS_NAME || fileName == S_POS2D_INTERCOLOR_PS_NAME)
			return CMImplementedShaderType::POS2D_INTERCOLOR;

		return CMImplementedShaderType::INVALID;
	}

	[[nodiscard]] void CMShaderLibrary::GetCorrespondingDescription(Utility::CMStaticArray<D3D11_INPUT_ELEMENT_DESC>& outDesc, CMImplementedShaderType implementedType) const noexcept
	{
		switch (implementedType)
		{
		case CMImplementedShaderType::POS2D_INTERCOLOR:
		{
			D3D11_INPUT_ELEMENT_DESC descs[] = {
				{ "InterColor", 0u, DXGI_FORMAT_R8G8B8A8_UNORM, 0u, 0u, D3D11_INPUT_PER_VERTEX_DATA, 0u },
				{ "Pos2D", 0u, DXGI_FORMAT_R32G32_UINT, 0u, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0u}
			};

			outDesc.Set(descs);
			return;
		}
		case CMImplementedShaderType::DEFAULT:
		{
			D3D11_INPUT_ELEMENT_DESC descs[] = {
				{ "Pos2D", 0u, DXGI_FORMAT_R32G32_FLOAT, 0u, 0u, D3D11_INPUT_PER_VERTEX_DATA, 0u }
			};

			outDesc.Set(descs);
			return;
		}
		case CMImplementedShaderType::DEFAULT3D:
		{
			D3D11_INPUT_ELEMENT_DESC descs[] = {
				{ "Pos3D", 0u, DXGI_FORMAT_R32G32B32_FLOAT, 0u, 0u, D3D11_INPUT_PER_VERTEX_DATA, 0u }
			};

			outDesc.Set(descs);
			return;
		}
		default:
			return;
		}
	}
}