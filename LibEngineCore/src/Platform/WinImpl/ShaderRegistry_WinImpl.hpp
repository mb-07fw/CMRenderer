#pragma once

#include "Platform/Core/Shader.hpp"
#include "Platform/WinImpl/Types_WinImpl.hpp"

#include <vector>
#include <unordered_map>
#include <string_view>
#include <string>
#include <filesystem>
#include <type_traits>

#include <d3d11.h>

namespace CMEngine::Platform::WinImpl
{
	struct ShaderData
	{
		inline ShaderData(ShaderID id, ComPtr<ID3DBlob> pBytecode, const std::filesystem::path& fileName) noexcept
			: ID(id),
			  pBytecode(pBytecode),
			  Filename(fileName)
		{
		}

		ShaderData() = default;
		~ShaderData() = default;

		ShaderID ID;
		ComPtr<ID3DBlob> pBytecode;
		std::filesystem::path Filename; /* (no extension) */
	};

	template <typename InterfaceTy>
	struct Shader
	{
		inline Shader(ShaderID id, const ComPtr<InterfaceTy>& pShader) noexcept
			: ID(id),
			  pShader(pShader)
		{
		}

		inline void Bind(const ComPtr<ID3D11DeviceContext>& pContext) const noexcept
		{
			if constexpr (std::is_same_v<InterfaceTy, ID3D11VertexShader>)
				pContext->VSSetShader(pShader.Get(), nullptr, 0);
			else if constexpr (std::is_same_v<InterfaceTy, ID3D11PixelShader>)
				pContext->PSSetShader(pShader.Get(), nullptr, 0);
			else
				static_assert(false, "No corresponding binding functionality for InterfaceTy");
		}

		ShaderID ID;
		ComPtr<InterfaceTy> pShader;
	};

	using VertexShader = Shader<ID3D11VertexShader>;
	using PixelShader = Shader<ID3D11PixelShader>;
	using ComputeShader = Shader<ID3D11ComputeShader>;

	class ShaderRegistry
	{
	public:
		ShaderRegistry() noexcept;
		~ShaderRegistry() = default;

		ShaderRegistry(const ShaderRegistry&) = delete;
		ShaderRegistry(ShaderRegistry&&) = delete;
		ShaderRegistry& operator=(const ShaderRegistry&) = delete;
		ShaderRegistry& operator=(ShaderRegistry&&) = delete;
	public:
		void CreateShaders(const ComPtr<ID3D11Device>& pDevice) noexcept;
		void BindShader(ShaderID id, const ComPtr<ID3D11DeviceContext>& pContext) noexcept;

		void ClearBound() noexcept;

		[[nodiscard]] ShaderID QueryID(const std::wstring& shaderName) const noexcept;
		[[nodiscard]] const ShaderData* Retrieve(ShaderID id) const noexcept;

		inline [[nodiscard]] const std::vector<ShaderData>& Data() const noexcept { return m_ShaderData; }
		inline [[nodiscard]] ShaderID LastVS() const noexcept { return m_LastVS; }
		inline [[nodiscard]] ShaderID LastPS() const noexcept { return m_LastPS; }
	private:
		void LoadShaders() noexcept;
	private:
		static constexpr std::string_view S_COMPILED_SHADER_EXT = ".cso";
		static constexpr std::wstring_view S_COMPILED_SHADER_EXTW = L".cso";
		ShaderID m_LastVS;
		ShaderID m_LastPS;
		/* TODO: Come up with a more elegant way to accomplish this... */
		uint32_t m_NextShaderIndex = 0;
		std::vector<VertexShader> m_VertexShaders;
		std::vector<PixelShader> m_PixelShaders;
		std::vector<ShaderData> m_ShaderData;
		std::unordered_map<std::wstring, ShaderID> m_ShaderNames = {
			{ { L"Gltf_Basic_VS" }, { m_NextShaderIndex++, ShaderType::Vertex, AssignedShaderType::Gltf_Basic_VS } },
			{ { L"Gltf_Basic_PS" }, { m_NextShaderIndex++, ShaderType::Pixel,  AssignedShaderType::Gltf_Basic_PS } },
			{ { L"Quad_VS" }, { m_NextShaderIndex++, ShaderType::Vertex, AssignedShaderType::Quad_VS } },
			{ { L"Quad_PS" }, { m_NextShaderIndex++, ShaderType::Pixel,  AssignedShaderType::Quad_PS } }
		};
	};
}