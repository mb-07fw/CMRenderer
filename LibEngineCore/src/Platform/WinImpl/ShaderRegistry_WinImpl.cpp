#include "PCH.hpp"
#include "Platform/WinImpl/ShaderRegistry_WinImpl.hpp"

namespace CMEngine::Platform::WinImpl
{
	ShaderRegistry::ShaderRegistry() noexcept
	{
		LoadShaders();
	}

	void ShaderRegistry::CreateShaders(const ComPtr<ID3D11Device>& pDevice) noexcept
	{
		for (const ShaderData& data : m_ShaderData)
		{
			HRESULT hr = S_OK;

			switch (data.ID.Type)
			{
			default:				  [[fallthrough]];
			case ShaderType::Invalid: [[fallthrough]];
			case ShaderType::Compute:
				spdlog::critical("(ShaderRegistry) [CreateShaders] Failed to create shader for ShaderType in shader data list. (Unimplemented or invalid?)");
				break;
			case ShaderType::Vertex:
			{
				ComPtr<ID3D11VertexShader> pShader;
				hr = pDevice->CreateVertexShader(
					data.pBytecode->GetBufferPointer(),
					data.pBytecode->GetBufferSize(),
					nullptr,
					&pShader
				);

				if (!FAILED(hr))
					m_VertexShaders.emplace_back(data.ID, pShader);

				break;
			}
			case ShaderType::Pixel:
			{
				ComPtr<ID3D11PixelShader> pShader;
				hr = pDevice->CreatePixelShader(
					data.pBytecode->GetBufferPointer(),
					data.pBytecode->GetBufferSize(),
					nullptr,
					&pShader
				);

				if (!FAILED(hr))
					m_PixelShaders.emplace_back(data.ID, pShader);

				break;
			}
			}

			if (FAILED(hr))
				spdlog::critical("(ShaderRegistry) [CreateShaders] Failed to create shader for a shader type. Error: {}", hr);

		}
	}

	void ShaderRegistry::BindShader(ShaderID id, const ComPtr<ID3D11DeviceContext>& pContext) noexcept
	{
		switch (id.Type)
		{
		default: [[fallthrough]];
		case ShaderType::Invalid: [[fallthrough]];
		case ShaderType::Compute:
			spdlog::warn("(ShaderRegistry) [BindShader] Attempted to bind an invalid or unregistered ShaderType.");
			break;
		case ShaderType::Vertex:
			for (const VertexShader& vertexShader : m_VertexShaders)
				if (vertexShader.ID.AssignedType == id.AssignedType)
				{
					vertexShader.Bind(pContext);
					break;
				}

			m_LastVS = id;
			break;
		case ShaderType::Pixel:
			for (const PixelShader& pixelShader : m_PixelShaders)
				if (pixelShader.ID.AssignedType == id.AssignedType)
				{
					pixelShader.Bind(pContext);
					break;
				}

			m_LastPS = id;
			break;
		}
	}

	[[nodiscard]] ShaderID ShaderRegistry::QueryID(const std::wstring& shaderName) const noexcept
	{
		auto it = m_ShaderNames.find(shaderName.data());

		if (it == m_ShaderNames.end())
			return ShaderID{};

		return it->second;
	}

	[[nodiscard]] const ShaderData* ShaderRegistry::Retrieve(ShaderID id) const noexcept
	{
		if (id >= m_ShaderData.size())
			return nullptr;

		return &m_ShaderData[id.Index];
	}

	void ShaderRegistry::LoadShaders() noexcept
	{
		if (!std::filesystem::exists(CM_SHADERS_SHADER_DIRECTORY))
			spdlog::critical(
				"(WinImpl_ShaderLibrary) Internal error: Shader directory doesn't exist. This indicates a fatal error in the build system. Expected directory: `"
				CM_SHADERS_SHADER_DIRECTORY "`"
			);
		else if (!std::filesystem::exists(CM_SHADERS_COMPILED_SHADER_DIRECTORY))
			spdlog::critical(
				"(WinImpl_ShaderLibrary) Internal error: Compiled shader directory doesn't exist. This indicates a fatal error in the build system. Expected directory: `"
				CM_SHADERS_COMPILED_SHADER_DIRECTORY "`"
			);

		for (const auto& entry : std::filesystem::directory_iterator(CM_SHADERS_COMPILED_SHADER_DIRECTORY))
		{
			if (!entry.exists())
			{
				spdlog::warn("(WinImpl_ShaderLibrary) Internal warning: Skipping nonexistent entry in compiled shader directory.");
				continue;
			}

			const std::filesystem::path& path = entry.path();
			std::filesystem::path extPath = path.extension();
			std::filesystem::path fileNamePath = path.filename();
			std::filesystem::path fileStemPath = path.stem();

			if (extPath != S_COMPILED_SHADER_EXT)
			{
				spdlog::info("(WinImpl_ShaderLibrary) Internal info: Skipping non-cso: `{}`", fileNamePath.generic_string());
				continue;
			}

			ShaderID id = QueryID(fileStemPath.c_str());

			if (!id)
			{
				spdlog::warn(
					"(WinImpl_ShaderLibrary) Internal warning: Un-assigned shader name found in compiled shader directory. Skipping file: `{}`",
					fileStemPath.generic_string()
				);
				continue;
			}

			ComPtr<ID3DBlob> pBytecode;
			HRESULT hr = D3DReadFileToBlob(path.c_str(), &pBytecode);

			if (FAILED(hr))
				spdlog::critical("(WinImpl_ShaderLibrary) Internal error: Failed to read in shader bytecode. Error code: {}", hr);

			if (m_ShaderData.size() <= id.Index)
				m_ShaderData.resize(id.Index + 1);

			m_ShaderData[id.Index] = std::move(ShaderData(id, pBytecode, fileStemPath));
		}
	}
}