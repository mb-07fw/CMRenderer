#pragma once

#include "Internal/CMLogger.hpp"
#include "Internal/CMRendererSettings.hpp"
#include "Internal/CMStaticArray.hpp"
#include "Internal/CMShaderData.hpp"

#include <d3d11.h>

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>
#include <span>

namespace CMRenderer
{
	class CMShaderLibrary
	{
	public:
		CMShaderLibrary(CMLoggerWide& cmLoggerRef) noexcept;
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
		static constexpr size_t S_EXPECTED_NUM_SHADERS = 4;
		static constexpr size_t S_EXPECTED_NUM_SHADER_SETS = 2;
		static constexpr std::wstring_view S_DEFAULT_VS_NAME = L"DefaultVS.cso";
		static constexpr std::wstring_view S_DEFAULT_PS_NAME = L"DefaultPS.cso";
		static constexpr std::wstring_view S_POS2D_COLOR_TRANSFORM_VS_NAME = L"Pos2D_Color_TransformVS.cso";
		static constexpr std::wstring_view S_POS2D_COLOR_TRANSFORM_PS_NAME = L"Pos2D_Color_TransformPS.cso";
		bool m_Initialized = false;
		bool m_Shutdown = false;
		CMLoggerWide& m_CMLoggerRef;
		std::wstring m_CompiledShaderDirectory;
		std::vector<CMShaderSet> m_ShaderSets;
	};
}