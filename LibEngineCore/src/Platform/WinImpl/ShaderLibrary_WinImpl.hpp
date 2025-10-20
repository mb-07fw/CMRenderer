#pragma once

#include "Platform/WinImpl/ShaderSet_WinImpl.hpp"

#include <d3d11.h>

#include <string_view>
#include <string>
#include <array>
#include <vector>
#include <unordered_map>
#include <memory>
#include <span>

namespace CMEngine::Platform::WinImpl
{
#pragma region Shader Registry
	class ShaderRegistry
	{
	public:
		ShaderRegistry() = default;
		~ShaderRegistry() = default;

		ShaderRegistry(const ShaderRegistry&) = delete;
		ShaderRegistry(ShaderRegistry&&) = delete;
		ShaderRegistry& operator=(const ShaderRegistry&) = delete;
		ShaderRegistry& operator=(ShaderRegistry&&) = delete;
	public:
		[[nodiscard]] ShaderID QueryID(const std::filesystem::path& fileName) const noexcept;

		void Add(ShaderID id, const ComPtr<ID3DBlob>& pBytecode, const std::filesystem::path& fileName) noexcept;

		[[nodiscard]] const ShaderData* Retrieve(ShaderID id) const noexcept;

		inline [[nodiscard]] const std::vector<ShaderData>& Data() const noexcept { return m_ShaderData; }
	private:
		uint32_t m_NextShaderIndex = 0;
		std::vector<ShaderData> m_ShaderData;
		std::unordered_map<std::wstring, ShaderID> m_ShaderNames = {
			{ { L"GLTF_VS" }, { m_NextShaderIndex++, ShaderType::VERTEX, AssignedShaderType::GLTF_VS, ShaderSetType::GLTF } },
			{ { L"GLTF_PS" }, { m_NextShaderIndex++, ShaderType::PIXEL,  AssignedShaderType::GLTF_PS, ShaderSetType::GLTF } },
			{ { L"Quad_VS" }, { m_NextShaderIndex++, ShaderType::VERTEX, AssignedShaderType::QUAD_VS, ShaderSetType::QUAD } },
			{ { L"Quad_PS" }, { m_NextShaderIndex++, ShaderType::PIXEL,  AssignedShaderType::QUAD_PS, ShaderSetType::QUAD } }
		};
	};
#pragma endregion

#pragma region Shader Library
	class ShaderLibrary
	{
	public:
		ShaderLibrary() noexcept;
		~ShaderLibrary() noexcept;
	public:
		void CreateShaderSets(const ComPtr<ID3D11Device>& pDevice) noexcept;
		void BindSet(ShaderSetType setType, const ComPtr<ID3D11DeviceContext>& pContext) noexcept;
	private:
		void LoadShaders() noexcept;

		[[nodiscard]] bool ValidateSetSupportsUse(ShaderSetType setType, ShaderType type, const std::filesystem::path& fileName, ShaderUseID useID) noexcept;
	private:
		static constexpr std::string_view S_COMPILED_SHADER_EXT = ".cso";
		static constexpr std::wstring_view S_COMPILED_SHADER_EXTW = L".cso";
		ShaderRegistry m_Registry;
		bool m_CreatedShaderSets = false;
		std::vector<std::shared_ptr<IShaderSet>> m_ShaderSets;
		ShaderSetType m_CurrentSet = ShaderSetType::INVALID;
	};
#pragma endregion
}