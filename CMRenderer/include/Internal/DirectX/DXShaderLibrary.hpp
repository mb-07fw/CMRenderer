#pragma once

#include "Internal/Utility/CMLogger.hpp"
#include "Internal/Utility/CMStaticArray.hpp"
#include "Internal/CMRendererSettings.hpp"
#include "Internal/DirectX/DXShaderData.hpp"

#include <d3d11.h>

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>
#include <span>

namespace CMRenderer::CMDirectX
{
	class CMShaderLibrary
	{
	public:
		CMShaderLibrary(Utility::CMLoggerWide& cmLoggerRef) noexcept;
		~CMShaderLibrary() noexcept;
	public:
		void Init() noexcept;
		void Shutdown() noexcept;

		[[nodiscard]] const CMShaderSet& GetSetOfType(CMImplementedShaderType implementedType) noexcept;

		inline [[nodiscard]] bool IsInitialized() const noexcept { return m_Initialized; }
		inline [[nodiscard]] bool IsShutdown() const noexcept { return m_Shutdown; }
	private:
		void GetCompiledShaderDirectory(std::filesystem::path& outPathRef) noexcept;
		void GetAllShaderData(std::vector<CMShaderData>& outDataRef, const std::filesystem::path& compiledShaderPathRef) noexcept;

		[[nodiscard]] size_t TotalCompiledShaders(const std::filesystem::path& compiledShaderPathRef) const noexcept;
		[[nodiscard]] CMImplementedShaderType CorrespondingImplementedType(const std::wstring& fileName) const noexcept;
		[[nodiscard]] void GetCorrespondingDescription(Utility::CMStaticArray<D3D11_INPUT_ELEMENT_DESC>& outDesc, CMImplementedShaderType implementedType) const noexcept;
	private:
		static constexpr size_t S_EXPECTED_NUM_SHADERS = 6;
		static constexpr size_t S_EXPECTED_NUM_SHADER_SETS = 3;
		static constexpr std::wstring_view S_DEFAULT_VS_NAME = L"DefaultVS.cso";
		static constexpr std::wstring_view S_DEFAULT_PS_NAME = L"DefaultPS.cso";
		static constexpr std::wstring_view S_DEFAULT3D_VS_NAME = L"Default3D_VS.cso";
		static constexpr std::wstring_view S_DEFAULT3D_PS_NAME = L"Default3D_PS.cso";
		static constexpr std::wstring_view S_POS2D_INTERCOLOR_VS_NAME = L"Pos2D_InterColorVS.cso";
		static constexpr std::wstring_view S_POS2D_INTERCOLOR_PS_NAME = L"Pos2D_InterColorPS.cso";
		Utility::CMLoggerWide& m_CMLoggerRef;
		std::wstring m_CompiledShaderDirectory;
		std::vector<CMShaderSet> m_ShaderSets;
		bool m_Initialized = false;
		bool m_Shutdown = false;
	};
}