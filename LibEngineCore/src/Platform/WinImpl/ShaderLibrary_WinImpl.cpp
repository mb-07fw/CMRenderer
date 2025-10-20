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

	void ShaderRegistry::Add(ShaderID id, const ComPtr<ID3DBlob>& pBytecode, const std::filesystem::path& fileName) noexcept
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

#pragma region Shader Library
	ShaderLibrary::ShaderLibrary() noexcept
	{
		LoadShaders();
	}

	ShaderLibrary::~ShaderLibrary() noexcept
	{
	}

	void ShaderLibrary::CreateShaderSets(const ComPtr<ID3D11Device>& pDevice) noexcept
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
				case ShaderType::INVALID: [[fallthrough]];
				default:
					spdlog::warn(
						"(WinImpl_ShaderLibrary) Internal warning: Collected shader data has an "
						"invalid shader type. Skipping data. Type: {}",
						static_cast<int16_t>(shaderData.ID.Type)
					);

					break;
				case ShaderType::VERTEX:
					if (!ValidateSetSupportsUse(setType, shaderData.ID.Type, shaderData.Filename, (ShaderUseID)ShaderUseType::VERTEX))
						break;
					else if (pVSData != nullptr)
					{
						spdlog::warn(
							"(WinImpl_ShaderLibrary) Internal warning: Found more than one ShaderData "
							"of the same ShaderType for the current ShaderSetType. Skipping duplicate data. "
							"ShaderType: {}, ShaderSetType: {}, Filename: {}",
							static_cast<int16_t>(ShaderType::VERTEX),
							static_cast<int16_t>(setType),
							shaderData.Filename.generic_string()
						);

						break;
					}

					pVSData = &shaderData;
					validData = true;
					currentUseID |= (ShaderUseID)ShaderUseType::VERTEX;
					break;
				case ShaderType::PIXEL:
					if (!ValidateSetSupportsUse(setType, shaderData.ID.Type, shaderData.Filename, (ShaderUseID)ShaderUseType::PIXEL))
						break;
					else if (pPSData != nullptr)
					{
						spdlog::warn(
							"(WinImpl_ShaderLibrary) Internal warning: Found more than one ShaderData "
							"of the same ShaderType for the current ShaderSetType. Skipping duplicate data. "
							"ShaderType: {}, ShaderSetType: {}, Filename: {}",
							static_cast<int16_t>(ShaderType::PIXEL),
							static_cast<int16_t>(setType),
							shaderData.Filename.generic_string()
						);

						break;
					}

					pPSData = &shaderData;
					validData = true;
					currentUseID |= (ShaderUseID)ShaderUseType::PIXEL;
					break;
				case ShaderType::COMPUTE:
					if (!ValidateSetSupportsUse(setType, shaderData.ID.Type, shaderData.Filename, (ShaderUseID)ShaderUseType::COMPUTE))
						break;
					else if (pCSData != nullptr)
					{
						spdlog::warn(
							"(WinImpl_ShaderLibrary) Internal warning: Found more than one ShaderData "
							"of the same ShaderType for the current ShaderSetType. Skipping duplicate data. "
							"ShaderType: {}, ShaderSetType: {}, Filename: {}",
							static_cast<int16_t>(ShaderType::COMPUTE),
							static_cast<int16_t>(setType),
							shaderData.Filename.generic_string()
						);

						break;
					}

					pCSData = &shaderData;
					validData = true;
					currentUseID |= (ShaderUseID)ShaderUseType::COMPUTE;
					break;
				}

				if (!validData)
					continue;

				/* Continue collecting shader data until we have all the data the set uses.*/
				if (!IShaderSet::IsUseIDOf(setType, currentUseID))
					continue;

				switch (setType)
				{
				case ShaderSetType::INVALID: [[fallthrough]];
				case ShaderSetType::TOTAL_SETS: [[fallthrough]];
				default:
					spdlog::warn("(WinImpl_ShaderLibrary) Internal warning: Current ShaderSetType has a non-literal (invalid or sentinel) value. You did something stupid if this happened.");
					break;
				case ShaderSetType::GLTF:
					m_ShaderSets.emplace_back(std::make_shared<ShaderSetGltf>(pVSData->ID, pPSData->ID));
					collectedData = true;
					break;
				case ShaderSetType::QUAD:
					m_ShaderSets.emplace_back(std::make_shared<ShaderSetQuad>(pVSData->ID, pPSData->ID));
					collectedData = true;
					break;
				}

				if (collectedData)
					break;
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

	void ShaderLibrary::BindSet(ShaderSetType setType, const ComPtr<ID3D11DeviceContext>& pContext) noexcept
	{
		if (setType == m_CurrentSet)
			return;

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
				spdlog::info("(WinImpl_ShaderLibrary) Internal info: Skipping non-compiled shader file: `{}`", fileNamePath.generic_string());
				continue;
			}

			/* As of now, every shader in this directory is assumed to have a corresponding shader set. 
			 * The distinction between assigned and custom shader sets has yet to've been made. */
			ShaderID id = m_Registry.QueryID(fileStemPath);
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

			m_Registry.Add(id, pBytecode, fileStemPath);
		}
	}

	[[nodiscard]] bool ShaderLibrary::ValidateSetSupportsUse(ShaderSetType setType, ShaderType type, const std::filesystem::path& fileName, ShaderUseID useID) noexcept
	{
		if (IShaderSet::SupportsShaderUse(setType, useID))
			return true;
		
		spdlog::warn(
			"(WinImpl_ShaderLibrary) Internal warning: "
			"ShaderSetType `{}` of the collected ShaderData doesn't support it's ShaderUseType. "
			"Skipping the creation of the ShaderSetType. "
			"ShaderType: {}, Filename: {}, ShaderUseID: {}",
			static_cast<int16_t>(setType),
			static_cast<int16_t>(type),
			fileName.generic_string(),
			static_cast<int16_t>(useID)
		);
		return false;
	}
#pragma endregion
}