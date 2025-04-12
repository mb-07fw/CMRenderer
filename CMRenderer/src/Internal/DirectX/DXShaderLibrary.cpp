#include "Core/CMPCH.hpp"
#include "Internal/DirectX/DXShaderLibrary.hpp"

namespace CMRenderer::CMDirectX
{
	DXShaderLibrary::DXShaderLibrary(Utility::CMLoggerWide& cmLoggerRef) noexcept
		: m_CMLoggerRef(cmLoggerRef)
	{
		LoadShaders();

		m_CMLoggerRef.LogInfoNL(L"CMShaderLibrary [()] | Constructed.");
	}

	DXShaderLibrary::~DXShaderLibrary() noexcept
	{
		if (m_Initialized)
			Shutdown();

		m_CMLoggerRef.LogInfoNL(L"CMShaderLibrary [~()] | Destroyed.");
	}

	void DXShaderLibrary::Init(Components::DXDevice& deviceRef) noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(m_Initialized, L"CMShaderLibrary [Init] | Initialization has been attempted after initialization already occured.");

		CreateAllShaders(deviceRef);

		m_Initialized = true;
		m_Shutdown = false;
		m_CMLoggerRef.LogInfoNL(L"CMShaderLibrary [Init] | Initialized.");
	}

	void DXShaderLibrary::Shutdown() noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(!m_Initialized, L"CMShaderLibrary [Shutdown] | Shutdown has been attempted before initialization has occured.");
		m_CMLoggerRef.LogFatalNLIf(m_Shutdown, L"CMShaderLibrary [Shutdown] | Shutdown has been attempted after shutdown has already occured.");

		m_CompiledShaderDirectory.clear();
		m_ShaderSets.clear();

		m_Initialized = false;
		m_Shutdown = true;
		m_CMLoggerRef.LogInfoNL(L"CMShaderLibrary [Shutdown] | Shutdown.");
	}

	[[nodiscard]] DXShaderSet& DXShaderLibrary::GetSetOfType(DXImplementedShaderType implementedType) noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(!m_Initialized, L"CMShaderLibrary [GetSetOfType] | Attempted to retrieve a shader set before initialization occured.");

		for (DXShaderSet& set : m_ShaderSets)
			if (set.ImplementedType() == implementedType)
				return set;

		m_CMLoggerRef.LogFatalNL(L"CMShaderLibrary [GetSetOfType] | Couldn't find a shader set with the matching implemented type.");
		return m_ShaderSets.back();
	}

	void DXShaderLibrary::LoadShaders() noexcept
	{
		m_ShaderSets.reserve(S_EXPECTED_NUM_SHADER_SETS);

		std::filesystem::path compiledShaderPath;
		GetCompiledShaderDirectory(compiledShaderPath);

		std::vector<DXShaderData> shaderData;
		GetAllShaderData(shaderData, compiledShaderPath);

		std::wstring message;
		for (DXImplementedShaderType implementedType : G_IMPLEMENTED_SHADER_TYPES)
		{
			const DXShaderData* pVertexData = nullptr;
			const DXShaderData* pPixelData = nullptr;

			for (const DXShaderData& data : shaderData)
			{
				if (data.ImplementedType != implementedType)
					continue;

				switch (data.Type)
				{
				case DXShaderType::INVALID:
					m_CMLoggerRef.LogFatalNL(L"CMShaderLibrary [Init] | A collected shader has an invalid CMShaderType.");
				case DXShaderType::VERTEX:
					if (pVertexData != nullptr)
						m_CMLoggerRef.LogFatalNL(L"CMShaderLibrary [Init] | A second vertex shader was found for the same CMImplementedShaderType.");

					pVertexData = &data;
					break;
				case DXShaderType::PIXEL:
					if (pVertexData != nullptr)
						m_CMLoggerRef.LogFatalNL(L"CMShaderLibrary [Init] | A second pixel shader was found for the same CMImplementedShaderType.");

					pPixelData = &data;
					break;
				default:
					m_CMLoggerRef.LogFatalNL(L"CMShaderLibrary [Init] | No matching cases found for the CMShaderType of a shader.");
				}
			}

			m_CMLoggerRef.LogFatalNLAppendIf(
				pVertexData == nullptr,
				L"CMShaderLibrary [Init] | No vertex shader was found for the CMImplementedType : ",
				DXShaderData::ImplementedToWStrView(implementedType)
			);

			m_CMLoggerRef.LogFatalNLAppendIf(
				pPixelData == nullptr,
				L"CMShaderLibrary [Init] | No pixel shader was found for the CMImplementedType : ",
				DXShaderData::ImplementedToWStrView(implementedType)
			);

			std::vector<D3D11_INPUT_ELEMENT_DESC> desc;
			GetCorrespondingDescription(desc, implementedType);

			m_ShaderSets.emplace_back(*pVertexData, *pPixelData, desc, implementedType);
		}

		m_CMLoggerRef.LogFatalNLVariadicIf(
			(m_ShaderSets.size() != S_EXPECTED_NUM_SHADER_SETS),
			L"CMShaderLibrary [Init] | Failed to collect the expected amount of shader sets.",
			L"(Collected : ", m_ShaderSets.size(),
			L" | Expected : ", S_EXPECTED_NUM_SHADER_SETS
		);
	}

	void DXShaderLibrary::CreateAllShaders(Components::DXDevice& deviceRef) noexcept
	{
		for (DXShaderSet& shaderSet : m_ShaderSets)
			shaderSet.CreateShaders(deviceRef, m_CMLoggerRef);
	}

	void DXShaderLibrary::GetCompiledShaderDirectory(std::filesystem::path& outPathRef) noexcept
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

		m_CMLoggerRef.LogFatalNLIf(!std::filesystem::exists(outPathRef), L"CMShaderLibrary [GetCompiledShaderDirectory] | Shader directory doesn't exist.");
		m_CMLoggerRef.LogFatalNLIf(!std::filesystem::is_directory(outPathRef), L"CMShaderLibrary [GetCompiledShaderDirectory] | Shader directory wasn't a directory.");
	}

	void DXShaderLibrary::GetAllShaderData(std::vector<DXShaderData>& outDataRef, const std::filesystem::path& compiledShaderPathRef) noexcept
	{
		outDataRef.reserve(S_EXPECTED_NUM_SHADERS);
		size_t totalShadersPresent = TotalCompiledShaders(compiledShaderPathRef);

		std::wstring message;

		
		m_CMLoggerRef.LogFatalNLIf(totalShadersPresent == 0, L"CMShaderLibrary [GetAllShaderData] | No shaders were found.");

		m_CMLoggerRef.LogFatalNLVariadicIf(
			totalShadersPresent != S_EXPECTED_NUM_SHADERS,
			L"CMShaderLibrary [GetAllShaderData] | The amount of compiled shaders didn't match the expected amount.",
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
				L"CMShaderLibrary [GetAllShaderData] | Invalid shader : ",
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
					L"CMShaderLibrary [GetAllShaderData] | Invalid shader : ",
					fileName, ". Expected flag in shader name. (\"VS\", \"PS\")"
				);

			HRESULT hResult = S_OK;

			m_CMLoggerRef.LogFatalNLAppendIf(
				(hResult = D3DReadFileToBlob(entryWStr.data(), &pBlob)) != S_OK,
				L"CMShaderLibrary [GetAllShaderData] | Failed to read in compiled shader : ",
				fileName
			);

			DXImplementedShaderType implementedType = CorrespondingImplementedType(fileName);
			
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
				DXShaderData::ImplementedToWStrView(implementedType)
			);

			m_CMLoggerRef.LogFatalNLAppendIf(
				implementedType == DXImplementedShaderType::INVALID,
				L"CMShaderLibrary [GetAllShaderData] | Failed to find a matching implemented type for the shader : ",
				fileName
			);
			
			outDataRef.emplace_back(implementedType, shaderType, pBlob, fileName);
		}

		m_CMLoggerRef.LogInfoNLAppend(
			L"CMShaderLibrary [GetAllShaderData] | Total shaders collected : ", 
			outDataRef.size()
		);

		m_CMLoggerRef.LogFatalNLVariadicIf(
			outDataRef.size() != S_EXPECTED_NUM_SHADERS,
			L"CMShaderLibrary [GetAllShaderData] | Failed to collect the expected amount of shaders.",
			L"(Collected : ", outDataRef.size(),
			L" | Expected : ", S_EXPECTED_NUM_SHADERS
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

	[[nodiscard]] DXImplementedShaderType DXShaderLibrary::CorrespondingImplementedType(const std::wstring& fileName) const noexcept
	{
		if (fileName == S_DEFAULT_VS_NAME || fileName == S_DEFAULT_PS_NAME)
			return DXImplementedShaderType::DEFAULT;
		else if (fileName == S_DEFAULT3D_VS_NAME || fileName == S_DEFAULT3D_PS_NAME)
			return DXImplementedShaderType::DEFAULT3D;
		else if (fileName == S_DEFAULT_TEXTURE_VS_NAME || fileName == S_DEFAULT_TEXTURE_PS_NAME)
			return DXImplementedShaderType::DEFAULT_TEXTURE;
		else if (fileName == S_POS2D_INTERCOLOR_VS_NAME || fileName == S_POS2D_INTERCOLOR_PS_NAME)
			return DXImplementedShaderType::POS2D_INTERCOLOR;

		return DXImplementedShaderType::INVALID;
	}

	[[nodiscard]] void DXShaderLibrary::GetCorrespondingDescription(std::vector<D3D11_INPUT_ELEMENT_DESC>& outDesc, DXImplementedShaderType implementedType) const noexcept
	{
		switch (implementedType)
		{
		case DXImplementedShaderType::POS2D_INTERCOLOR:
		{
			D3D11_INPUT_ELEMENT_DESC descs[] = {
				{ "InterColor", 0u, DXGI_FORMAT_R8G8B8A8_UNORM, 0u, 0u, D3D11_INPUT_PER_VERTEX_DATA, 0u },
				{ "Pos2D", 0u, DXGI_FORMAT_R32G32_UINT, 0u, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0u}
			};

			outDesc.assign(std::begin(descs), std::end(descs));
			return;
		}
		case DXImplementedShaderType::DEFAULT:
		{
			D3D11_INPUT_ELEMENT_DESC descs[] = {
				{ "Pos2D", 0u, DXGI_FORMAT_R32G32_FLOAT, 0u, 0u, D3D11_INPUT_PER_VERTEX_DATA, 0u }
			};

			outDesc.assign(std::begin(descs), std::end(descs));
			return;
		}
		case DXImplementedShaderType::DEFAULT3D:
		{
			D3D11_INPUT_ELEMENT_DESC descs[] = {
				{ "Pos3D", 0u, DXGI_FORMAT_R32G32B32_FLOAT, 0u, 0u, D3D11_INPUT_PER_VERTEX_DATA, 0u }
			};

			outDesc.assign(std::begin(descs), std::end(descs));
			return;
		}
		case DXImplementedShaderType::DEFAULT_TEXTURE:
		{
			D3D11_INPUT_ELEMENT_DESC descs[] = {
				{ "VertexPos3D", 0u, DXGI_FORMAT_R32G32B32_FLOAT, 0u, 0u, D3D11_INPUT_PER_VERTEX_DATA, 0u },
				{ "TexCoord", 0u, DXGI_FORMAT_R32G32_FLOAT, 0u, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0u }
			};

			outDesc.assign(std::begin(descs), std::end(descs));
			return;
		}
		default:
			return;
		}
	}
}