#include "Core/CMPCH.hpp"
#include "Internal/CMShaderLibrary.hpp"

namespace CMRenderer
{
	CMShaderLibrary::CMShaderLibrary(CMLoggerWide& cmLoggerRef) noexcept
		: m_CMLoggerRef(cmLoggerRef)
	{
		m_ShaderSets.reserve(S_EXPECTED_NUM_SHADER_SETS);
		m_CMLoggerRef.LogInfo(L"CMShaderLibrary [()] | Constructed.\n");
	}

	CMShaderLibrary::~CMShaderLibrary() noexcept
	{
		if (m_Initialized)
			Shutdown();

		m_CMLoggerRef.LogInfo(L"CMShaderLibrary [~()] | Destroyed.\n");
	}

	void CMShaderLibrary::Init() noexcept
	{
		if (m_Initialized)
		{
			m_CMLoggerRef.LogWarning(L"CMShaderLibrary [Init] | Initialization has been attempted after initialization already occured.\n");
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
					m_CMLoggerRef.LogFatal(L"CMShaderLibrary [Init] | A collected shader has an invalid CMShaderType.\n");
					break; // LogFatal will terminate the program anyway...
				case CMShaderType::VERTEX:
					if (pVertexData != nullptr)
						m_CMLoggerRef.LogFatal(L"CMShaderLibrary [Init] | A second vertex shader was found for the same CMImplementedShaderType.\n");

					pVertexData = &data;
					break;
				case CMShaderType::PIXEL:
					if (pVertexData != nullptr)
						m_CMLoggerRef.LogFatal(L"CMShaderLibrary [Init] | A second pixel shader was found for the same CMImplementedShaderType.\n");

					pPixelData = &data;
					break;
				default:
					m_CMLoggerRef.LogFatal(L"CMShaderLibrary [Init] | No matching cases found for the CMShaderType of a shader.\n");
				}
			}

			if (pVertexData == nullptr)
			{
				message = L"CMShaderLibrary [Init] | No vertex shader was found for the CMImplementedType : " + std::wstring(CMShaderData::ImplementedToWStr(implementedType)) + L"\n";
				m_CMLoggerRef.LogFatal(message);
				return; // Here so Intelisense doesn't yell at me for dereferencing a null pointer.
			}
			else if (pPixelData == nullptr)
			{
				message = L"CMShaderLibrary [Init] | No pixel shader was found for the CMImplementedType : " + std::wstring(CMShaderData::ImplementedToWStr(implementedType)) + L"\n";
				m_CMLoggerRef.LogFatal(message);
				return; // Here so Intelisense doesn't yell at me for dereferencing a null pointer.
			}

			Utility::CMStaticArray<D3D11_INPUT_ELEMENT_DESC> desc;
			GetCorrespondingDescription(desc, implementedType);

			m_ShaderSets.emplace_back(*pVertexData, *pPixelData, desc, implementedType);
		}

		if (m_ShaderSets.size() != S_EXPECTED_NUM_SHADER_SETS)
		{
			message = L"CMShaderLibrary [Init] | Failed to collect the expected amount of shader sets. (Collected : " +
				std::to_wstring(m_ShaderSets.size()) + L" | Expected : " + std::to_wstring(S_EXPECTED_NUM_SHADER_SETS) + L")\n";

			m_CMLoggerRef.LogFatal(message);
		}

		m_Initialized = true;
		m_Shutdown = false;
		m_CMLoggerRef.LogInfo(L"CMShaderLibrary [Init] | Initialied.\n");
	}

	void CMShaderLibrary::Shutdown() noexcept
	{
		if (!m_Initialized)
		{
			m_CMLoggerRef.LogWarning(L"CMShaderLibrary [Shutdown] | Shutdown has been attempted before initialization has occured.\n");
			return;
		}
		else if (m_Shutdown)
		{
			m_CMLoggerRef.LogWarning(L"CMShaderLibrary [Shutdown] | Shutdown has been attempted after shutdown has already occured.\n");
			return;
		}

		m_CompiledShaderDirectory.clear();
		m_ShaderSets.clear();

		m_Initialized = false;
		m_Shutdown = true;
		m_CMLoggerRef.LogInfo(L"CMShaderLibrary [Shutdown] | Shutdown.\n");
	}

	[[nodiscard]] const CMShaderSet& CMShaderLibrary::GetSetOfType(CMImplementedShaderType implementedType) noexcept
	{
		if (!m_Initialized)
			m_CMLoggerRef.LogFatal(L"CMShaderLibrary [GetSetOfType] | Attempted to retrieve a shader set before initialization occured.\n");

		for (const CMShaderSet& set : m_ShaderSets)
			if (set.ImplementedType() == implementedType)
				return set;

		m_CMLoggerRef.LogFatal(L"CMShaderLibrary [GetSetOfType] | Couldn't find a shader set with the matching implemented type.\n");
		return m_ShaderSets.back();
	}

	void CMShaderLibrary::GetCompiledShaderDirectory(std::filesystem::path& outPathRef) noexcept
	{
		std::filesystem::path workingPath = std::filesystem::current_path();

		std::wstring message = L"CMShaderLibrary [GetCompiledShaderDirectory] | Working directory : " + workingPath.wstring() + L'\n';
		m_CMLoggerRef.LogInfo(message);

		outPathRef = workingPath.parent_path() / "build" / CM_CONFIG / "Out";
		m_CompiledShaderDirectory = outPathRef.wstring();

		message = L"CMShaderLibrary [GetCompiledShaderDirectory] | Compiled shader path : " + m_CompiledShaderDirectory + L'\n';
		m_CMLoggerRef.LogInfo(message);

		if (!std::filesystem::exists(outPathRef))
			m_CMLoggerRef.LogFatal(L"CMShaderLibrary [GetCompiledShaderDirectory] | Shader directory doesn't exist.\n");
		else if (!std::filesystem::is_directory(outPathRef))
			m_CMLoggerRef.LogFatal(L"CMShaderLibrary [GetCompiledShaderDirectory] | Shader directory wasn't a directory.\n");
	}

	void CMShaderLibrary::GetAllShaderData(std::vector<CMShaderData>& outDataRef, const std::filesystem::path& compiledShaderPathRef) noexcept
	{
		size_t totalShaders = TotalCompiledShaders(compiledShaderPathRef);
		outDataRef.reserve(totalShaders);

		std::wstring message;

		if (totalShaders == 0)
			m_CMLoggerRef.LogFatal(L"CMShaderLibrary [GetAllShaderData] | No shaders were found.\n");
		else if (totalShaders != S_EXPECTED_NUM_SHADERS)
		{
			message = L"CMShaderLibrary [GetAllShaderData] | The amount of compiled shaders didn't match the expected amount. (Collected : " +
				std::to_wstring(totalShaders) + L" | Expected : " + std::to_wstring(S_EXPECTED_NUM_SHADERS) + L")\n";

			m_CMLoggerRef.LogFatal(message);
		}

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
				message = L"CMShaderLibrary [GetAllShaderData] | Invalid shader : " + fileName + L'\n';
				m_CMLoggerRef.LogWarning(message);
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
			{
				message = L"CMShaderLibrary [GetAllShaderData] | Invalid shader : " + fileName + L'\n';
				m_CMLoggerRef.LogFatal(message);
				continue;
			}

			HRESULT hResult = D3DReadFileToBlob(entryWStr.data(), &pBlob);

			if (hResult != S_OK)
			{
				message = L"CMShaderLibrary [GetAllShaderData] | Failed to read in shader : " + fileName + L'\n';
				m_CMLoggerRef.LogFatal(message);
			}

			CMImplementedShaderType implementedType = CorrespondingImplementedType(fileName);
			
			message = L"CMShaderLibrary [GetAllShaderData] | Found shader : " + fileName + L'\n';
			m_CMLoggerRef.LogInfo(message);

			message = L"CMShaderLibrary [GetAllShaderData] | Shader flag : " + shaderFlag + L'\n';
			m_CMLoggerRef.LogInfo(message);

			message = L"CMShaderLibrary [GetAllShaderData] | Implemented type : " +
				std::wstring(CMShaderData::ImplementedToWStr(implementedType)) + L'\n';
			m_CMLoggerRef.LogInfo(message);

			if (implementedType == CMImplementedShaderType::INVALID)
			{
				message = L"CMShaderLibrary [GetAllShaderData] | Failed to find a matching implemented type for the shader : " +
					fileName + L'\n';

				m_CMLoggerRef.LogFatal(message);
			}
			
			outDataRef.emplace_back(implementedType, shaderType, pBlob, fileName);
		}

		message = L"CMShaderLibrary [GetAllShaderData] | Total shaders collected : " + std::to_wstring(outDataRef.size()) + L'\n';
		m_CMLoggerRef.LogInfo(message);

		if (outDataRef.size() != S_EXPECTED_NUM_SHADERS)
		{
			message = L"CMShaderLibrary [GetAllShaderData] | Failed to collect the expected amount of shaders. (Collected : " +
				std::to_wstring(outDataRef.size()) + L" | Expected : " + std::to_wstring(S_EXPECTED_NUM_SHADERS) + L")\n";

			m_CMLoggerRef.LogFatal(message);
		}
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
		else if (fileName == S_POS2D_COLOR_TRANSFORM_VS_NAME || fileName == S_POS2D_COLOR_TRANSFORM_PS_NAME)
			return CMImplementedShaderType::POS2D_COLOR_TRANSFORM;

		return CMImplementedShaderType::INVALID;
	}

	[[nodiscard]] void CMShaderLibrary::GetCorrespondingDescription(Utility::CMStaticArray<D3D11_INPUT_ELEMENT_DESC>& outDesc, CMImplementedShaderType implementedType) const noexcept
	{
		switch (implementedType)
		{
		case CMImplementedShaderType::POS2D_COLOR_TRANSFORM: [[fallthrough]];
		case CMImplementedShaderType::DEFAULT:
		{
			D3D11_INPUT_ELEMENT_DESC descs[] = {
				{ "POSITION", 0u, DXGI_FORMAT_R32G32_FLOAT, 0u, 0u, D3D11_INPUT_PER_VERTEX_DATA, 0u }
			};

			outDesc.Set(descs);
			return;
		}
		default:
			return;
		}
	}
}