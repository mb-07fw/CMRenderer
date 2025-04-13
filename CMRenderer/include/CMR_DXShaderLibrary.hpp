#pragma once

#include <d3d11.h>

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>
#include <type_traits>

#include "CMC_Logger.hpp"
#include "CMR_RendererSettings.hpp"
#include "CMR_DXShaderData.hpp"

namespace CMRenderer::CMDirectX
{
	class DXShaderLibrary
	{
	public:
		DXShaderLibrary(Utility::CMLoggerWide& cmLoggerRef) noexcept;
		~DXShaderLibrary() noexcept;
	public:
		void Init(Components::DXDevice& deviceRef) noexcept;
		void Shutdown() noexcept;

		[[nodiscard]] DXShaderSet& GetSetOfType(DXImplementedShaderType implementedType) noexcept;

		inline [[nodiscard]] bool IsInitialized() const noexcept { return m_Initialized; }
		inline [[nodiscard]] bool IsShutdown() const noexcept { return m_Shutdown; }
	private:
		void LoadShaders() noexcept;

		void CreateAllShaders(Components::DXDevice& deviceRef) noexcept;

		void GetCompiledShaderDirectory(std::filesystem::path& outPathRef) noexcept;
		void GetAllShaderData(std::vector<DXShaderData>& outDataRef, const std::filesystem::path& compiledShaderPathRef) noexcept;

		[[nodiscard]] size_t TotalCompiledShaders(const std::filesystem::path& compiledShaderPathRef) const noexcept;
		[[nodiscard]] DXImplementedShaderType CorrespondingImplementedType(const std::wstring& fileName) const noexcept;
		[[nodiscard]] void GetCorrespondingDescription(std::vector<D3D11_INPUT_ELEMENT_DESC>& outDesc, DXImplementedShaderType implementedType) const noexcept;
	private:
		static constexpr size_t S_EXPECTED_NUM_SHADERS = 8;
		static constexpr size_t S_EXPECTED_NUM_SHADER_SETS = 4;
		static constexpr std::wstring_view S_DEFAULT_VS_NAME = L"DefaultVS.cso";
		static constexpr std::wstring_view S_DEFAULT_PS_NAME = L"DefaultPS.cso";
		static constexpr std::wstring_view S_DEFAULT3D_VS_NAME = L"Default3D_VS.cso";
		static constexpr std::wstring_view S_DEFAULT3D_PS_NAME = L"Default3D_PS.cso";
		static constexpr std::wstring_view S_DEFAULT_TEXTURE_VS_NAME = L"DefaultTexture_VS.cso";
		static constexpr std::wstring_view S_DEFAULT_TEXTURE_PS_NAME = L"DefaultTexture_PS.cso";
		static constexpr std::wstring_view S_POS2D_INTERCOLOR_VS_NAME = L"Pos2D_InterColorVS.cso";
		static constexpr std::wstring_view S_POS2D_INTERCOLOR_PS_NAME = L"Pos2D_InterColorPS.cso";
		Utility::CMLoggerWide& m_CMLoggerRef;
		std::wstring m_CompiledShaderDirectory;
		std::vector<DXShaderSet> m_ShaderSets;
		bool m_Initialized = false;
		bool m_Shutdown = false;
	};
}