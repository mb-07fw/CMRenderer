#include "PCH.hpp"
#include "Platform/WinImpl/ShaderLibrary_WinImpl.hpp"

namespace CMEngine::Platform::WinImpl
{
	[[nodiscard]] ShaderID ShaderRegistry::QueryID(const std::filesystem::path& fileName) const noexcept
	{
		auto it = m_ShaderNames.find(fileName);
		
		if (it == m_ShaderNames.end())
			return ShaderID{};

		return it->second;
	}

	void ShaderRegistry::Add(ShaderID id, ComPtr<ID3DBlob> pBytecode, const std::filesystem::path& fileName) noexcept
	{
		if (ShaderID queried = QueryID(fileName); queried != id)
		{
			spdlog::warn("(WinImpl_ShaderRegistry) Internal warning: Provied ID doesn't match the queried ID for the provided file name. Skipping add of shader data.");
			return;
		}

		if (id.Index >= m_ShaderData.size())
			m_ShaderData.resize(static_cast<size_t>(id.Index) + 1);

		m_ShaderData[id.Index] = std::move(ShaderData(id, pBytecode, fileName));
	}

	[[nodiscard]] const ShaderData* ShaderRegistry::Retrieve(ShaderID id) const noexcept
	{
		if (id >= m_ShaderData.size())
			return nullptr;

		return &m_ShaderData[id.Index];
	}

#pragma region Shader Sets
	IShaderSet::IShaderSet(ShaderUseID useFlags) noexcept
		: m_UseID(useFlags)
	{
	}

	void IShaderSet::CreateVertexShader(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11VertexShader>& pOutVertexShader,
		const ShaderRegistry& registry,
		ShaderID id
	) noexcept
	{
		if (pDevice.Get() == nullptr)
			spdlog::critical("(WinImpl_IShaderSet) Internal error: Provided device is nullptr.");
	
		const ShaderData* pVSData = registry.Retrieve(id);

		if (pVSData == nullptr)
		{
			spdlog::critical("(WinImpl_IShaderSet) Internal error: Failed to retrieve a valid ShaderData from the provided ShaderID.");
			std::exit(-1);
		}

		HRESULT hr = pDevice->CreateVertexShader(
			pVSData->pBytecode->GetBufferPointer(),
			pVSData->pBytecode->GetBufferSize(),
			nullptr,
			&pOutVertexShader
		);

		if (FAILED(hr))
			spdlog::critical("(WinImpl_IShaderSet) Internal error: Failed to create vertex shader.");
	}

	void IShaderSet::CreatePixelShader(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11PixelShader>& pOutPixelShader,
		const ShaderRegistry& registry,
		ShaderID id
	) noexcept
	{
		if (pDevice.Get() == nullptr)
			spdlog::critical("(WinImpl_IShaderSet) Internal error: Provided device is nullptr.");

		const ShaderData* pPSData = registry.Retrieve(id);

		if (pPSData == nullptr)
		{
			spdlog::critical("(WinImpl_IShaderSet) Internal error: Failed to retrieve a valid ShaderData from the provided ShaderID.");
			std::exit(-1);
		}

		HRESULT hr = pDevice->CreatePixelShader(
			pPSData->pBytecode->GetBufferPointer(),
			pPSData->pBytecode->GetBufferSize(),
			nullptr,
			&pOutPixelShader
		);

		if (FAILED(hr))
			spdlog::critical("(WinImpl_IShaderSet) Internal error: Failed to create pixel shader.");
	}

	void IShaderSet::CreateVertexAndPixelShader(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11VertexShader>& pOutVertexShader,
		ComPtr<ID3D11PixelShader>& pOutPixelShader,
		const ShaderRegistry& registry,
		ShaderID vertexID,
		ShaderID pixelID
	) noexcept
	{
		CreateVertexShader(pDevice, pOutVertexShader, registry, vertexID);
		CreatePixelShader(pDevice, pOutPixelShader, registry, pixelID);
	}

	void IShaderSet::CreateInputLayout(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11InputLayout>& pOutInputLayout,
		std::span<const D3D11_INPUT_ELEMENT_DESC> inputDescs,
		const ShaderRegistry& registry,
		ShaderID vertexID
	) noexcept
	{
		if (inputDescs.data() == nullptr)
			spdlog::warn("(WinImpl_IShaderSet) Internal warning: Provided input-element-desc data is nullptr. Input layout creation will likely fail.");
		else if (inputDescs.size() == 0)
			spdlog::warn("(WinImpl_IShaderSet) Internal warning: Provided input-element-desc data is empty. Input layout creation will likely fail.");

		const ShaderData* pVSData = registry.Retrieve(vertexID);

		if (pVSData == nullptr)
		{
			spdlog::critical("(WinImpl_IShaderSet) Internal error: Failed to retrieve a valid ShaderData from the registry with the provided ID.");
			std::exit(-1);
		}

		HRESULT hr = pDevice->CreateInputLayout(
			inputDescs.data(),
			static_cast<UINT>(inputDescs.size()),
			pVSData->pBytecode->GetBufferPointer(),
			pVSData->pBytecode->GetBufferSize(),
			&pOutInputLayout
		);

		if (FAILED(hr))
			spdlog::critical("(WinImpl_IShaderSet) Internal error: Failed to create input layout. Error code: {}", hr);
	}

	void IShaderSet::BindVertexShader(ComPtr<ID3D11DeviceContext> pContext, ComPtr<ID3D11VertexShader> pVertexShader) noexcept
	{
		if (pContext.Get() == nullptr)
			spdlog::critical("(WinImpl_IShaderSet) Internal error: Provided device context is nullptr.");

		pContext->VSSetShader(pVertexShader.Get(), nullptr, 0);
	}

	void IShaderSet::BindPixelShader(ComPtr<ID3D11DeviceContext> pContext, ComPtr<ID3D11PixelShader> pPixelShader) noexcept
	{
		if (pContext.Get() == nullptr)
			spdlog::critical("(WinImpl_IShaderSet) Internal error: Provided device context is nullptr.");

		pContext->PSSetShader(pPixelShader.Get(), nullptr, 0);
	}

	void IShaderSet::BindInputLayout(ComPtr<ID3D11DeviceContext> pContext, ComPtr<ID3D11InputLayout> pInputLayout) noexcept
	{
		pContext->IASetInputLayout(pInputLayout.Get());
	}

	void ShaderSetQuad::Create(ComPtr<ID3D11Device> pDevice, const ShaderRegistry& registry) noexcept
	{
		CreateVertexAndPixelShader(
			pDevice,
			mP_VertexShader,
			mP_PixelShader,
			registry,
			m_VertexID,
			m_PixelID
		);

		constexpr D3D11_INPUT_ELEMENT_DESC InputLayoutDescs[] = {
			{ "Pos", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		CreateInputLayout(
			pDevice,
			mP_InputLayout,
			InputLayoutDescs,
			registry,
			m_VertexID
		);
	}

	void ShaderSetQuad::Bind(ComPtr<ID3D11DeviceContext> pContext) noexcept
	{
		BindVertexShader(pContext, mP_VertexShader);
		BindPixelShader(pContext, mP_PixelShader);
		BindInputLayout(pContext, mP_InputLayout);
	}
#pragma endregion

#pragma region Shader Library
	ShaderLibrary::ShaderLibrary() noexcept
	{
		LoadShaders();
	}

	ShaderLibrary::~ShaderLibrary() noexcept
	{
	}

	void ShaderLibrary::CreateShaderSets(ComPtr<ID3D11Device> pDevice) noexcept
	{
		if (pDevice.Get() == nullptr)
			spdlog::critical("(WinImpl_ShaderLibrary) Internal error: Provided device is nullptr.");

		for (ShaderSetType setType : G_IMPLEMENTED_SHADER_SETS)
		{
			bool collectedData = false;
			ShaderUseID currentUseID = 0;
			const ShaderData* pVSData = nullptr;
			const ShaderData* pPSData = nullptr;
			const ShaderData* pCSData = nullptr;

			for (const ShaderData& shaderData : m_Registry.Data())
			{
				if (shaderData.ID.SetType != setType)
					continue;

				bool validData = false;
				switch (shaderData.ID.Type)
				{
				case SHADER_TYPE_INVALID: [[fallthrough]];
				default:
					spdlog::warn(
						"(WinImpl_ShaderLibrary) Internal warning: Collected shader data has an "
						"invalid shader type. Skipping data. Type: {}",
						static_cast<int16_t>(shaderData.ID.Type)
					);

					break;
				case SHADER_TYPE_VERTEX:
					if (!ValidateSetSupportsUse(setType, shaderData.ID.Type, shaderData.Filename, SHADER_USE_VERTEX))
						break;
					else if (pVSData != nullptr)
					{
						spdlog::warn(
							"(WinImpl_ShaderLibrary) Internal warning: Found more than one ShaderData "
							"of the same ShaderType for the current ShaderSetType. Skipping duplicate data. "
							"ShaderType: {}, ShaderSetType: {}, Filename: {}",
							static_cast<int16_t>(SHADER_TYPE_VERTEX),
							static_cast<int16_t>(setType),
							shaderData.Filename.generic_string()
						);

						break;
					}

					pVSData = &shaderData;
					validData = true;
					currentUseID |= SHADER_USE_VERTEX;
					break;
				case SHADER_TYPE_PIXEL:
					if (!ValidateSetSupportsUse(setType, shaderData.ID.Type, shaderData.Filename, SHADER_USE_PIXEL))
						break;
					else if (pPSData != nullptr)
					{
						spdlog::warn(
							"(WinImpl_ShaderLibrary) Internal warning: Found more than one ShaderData "
							"of the same ShaderType for the current ShaderSetType. Skipping duplicate data. "
							"ShaderType: {}, ShaderSetType: {}, Filename: {}",
							static_cast<int16_t>(SHADER_TYPE_PIXEL),
							static_cast<int16_t>(setType),
							shaderData.Filename.generic_string()
						);

						break;
					}

					pPSData = &shaderData;
					validData = true;
					currentUseID |= SHADER_USE_PIXEL;
					break;
				case SHADER_TYPE_COMPUTE:
					if (!ValidateSetSupportsUse(setType, shaderData.ID.Type, shaderData.Filename, SHADER_USE_COMPUTE))
						break;
					else if (pCSData != nullptr)
					{
						spdlog::warn(
							"(WinImpl_ShaderLibrary) Internal warning: Found more than one ShaderData "
							"of the same ShaderType for the current ShaderSetType. Skipping duplicate data. "
							"ShaderType: {}, ShaderSetType: {}, Filename: {}",
							static_cast<int16_t>(SHADER_TYPE_COMPUTE),
							static_cast<int16_t>(setType),
							shaderData.Filename.generic_string()
						);

						break;
					}

					pCSData = &shaderData;
					validData = true;
					currentUseID |= SHADER_USE_COMPUTE;
					break;
				}

				if (!validData)
					continue;

				/* Continue collecting shader data until we have all the data the set uses.*/
				if (!IShaderSet::IsUseIDOf(setType, currentUseID))
					continue;

				switch (setType)
				{
				case SHADER_SET_TYPE_INVALID: [[fallthrough]];
				case SHADER_SET_TYPE_TOTAL_SETS: [[fallthrough]];
				default:
					spdlog::warn("(WinImpl_ShaderLibrary) Internal warning: Current ShaderSetType has a non-literal (invalid or sentinel) value. You did something stupid if this happened.");
					break;
				case SHADER_SET_TYPE_QUAD:
					m_ShaderSets.emplace_back(std::make_shared<ShaderSetQuad>(pVSData->ID, pPSData->ID));
					collectedData = true;
					break;
				}
			}

			if (!collectedData)
			{
				spdlog::warn("(WinImpl_ShaderLibrary) Internal warning: Failed to collect all data for the current ShaderSetType. ShaderSetType: {}", 
					static_cast<int16_t>(setType)
				);

				continue;
			}

			m_ShaderSets.back()->Create(pDevice, m_Registry);
		}

		m_CreatedShaderSets = true;
	}

	void ShaderLibrary::BindSet(ShaderSetType setType, ComPtr<ID3D11DeviceContext> pContext) noexcept
	{
		for (const std::shared_ptr<IShaderSet>& pSet : m_ShaderSets)
			if (pSet->Type() == setType)
				pSet->Bind(pContext);
	}

	void ShaderLibrary::LoadShaders() noexcept
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

		std::vector<ShaderData> shaderData;
		for (const auto& entry : std::filesystem::directory_iterator(CM_SHADERS_COMPILED_SHADER_DIRECTORY))
		{
			if (!entry.exists())
			{
				spdlog::warn("(WinImpl_ShaderLibrary) Internal warning: Skipping nonexistent entry in compiled shader directory.");
				continue;
			}

			const std::filesystem::path& path = entry.path();
			std::filesystem::path extPath = path.extension();

			if (extPath != S_COMPILED_SHADER_EXT)
			{
				spdlog::info("(WinImpl_ShaderLibrary) Internal info: Skipping non-compiled shader file: `{}`", path.generic_string());
				continue;
			}

			std::filesystem::path fileNamePath = path.stem();

			/* As of now, every shader in this directory is assumed to have a corresponding shader set. 
			 * The distinction between assigned and custom shader sets has yet to've been made. */
			ShaderID id = m_Registry.QueryID(fileNamePath);
			if (!id)
			{
				spdlog::warn(
					"(WinImpl_ShaderLibrary) Internal warning: Un-assigned shader name found in compiled shader directory. Skipping file: `{}`",
					fileNamePath.generic_string()
				);

				continue;
			}

			ComPtr<ID3DBlob> pBytecode;
			HRESULT hr = D3DReadFileToBlob(path.c_str(), &pBytecode);

			if (FAILED(hr))
				spdlog::critical("(WinImpl_ShaderLibrary) Internal error: Failed to read in shader bytecode. Error code: {}", hr);

			m_Registry.Add(id, pBytecode, fileNamePath);
		}
	}

	[[nodiscard]] bool ShaderLibrary::ValidateSetSupportsUse(ShaderSetType setType, ShaderType type, const std::filesystem::path& fileName, ShaderUseID useID) noexcept
	{
		if (IShaderSet::SupportsShaderUse(setType, SHADER_USE_VERTEX))
			return true;
		
		spdlog::warn(
			"(WinImpl_ShaderLibrary) Internal warning: "
			"ShaderSetType `{}` of the collected ShaderData doesn't support it's ShaderType. "
			"Skipping the creation of the ShaderSetType. "
			"ShaderType: {}, Filename: {}",
			static_cast<int16_t>(setType),
			static_cast<int16_t>(type),
			fileName.generic_string()
		);
		return false;
	}
#pragma endregion
}