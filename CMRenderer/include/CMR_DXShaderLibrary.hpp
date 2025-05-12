#pragma once

#include <d3d11.h>

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>
#include <type_traits>

#include "CMC_Logger.hpp"
#include "CMR_RendererSettings.hpp"
#include "CMR_DXShaderSet.hpp"

namespace CMRenderer::CMDirectX
{
	class DXShaderLibrary
	{
	public:
		DXShaderLibrary(CMCommon::CMLoggerWide& cmLoggerRef) noexcept;
		~DXShaderLibrary() noexcept;
	public:
		void Init(Components::DXDevice& deviceRef) noexcept;
		void Shutdown() noexcept;

		[[nodiscard]] std::weak_ptr<IDXShaderSet> GetSetOfType(DXShaderSetType setType) noexcept;

		inline [[nodiscard]] bool IsInitialized() const noexcept { return m_Initialized; }
		inline [[nodiscard]] bool IsShutdown() const noexcept { return m_Shutdown; }
	private:
		void LoadShaders() noexcept;

		void CreateShaderSets(Components::DXDevice& deviceRef) noexcept;

		void GetCompiledShaderDirectory(std::filesystem::path& outPathRef) noexcept;
		void GetAllShaderData(std::vector<DXShaderData>& outDataRef, const std::filesystem::path& compiledShaderPathRef) noexcept;

		[[nodiscard]] size_t TotalCompiledShaders(const std::filesystem::path& compiledShaderPathRef) const noexcept;
		[[nodiscard]] DXShaderSetType CorrespondingSetType(const std::wstring& fileName) const noexcept;
	private:
		static constexpr size_t S_EXPECTED_NUM_SHADERS = 6;
		static constexpr size_t S_EXPECTED_NUM_SHADER_SETS = 3;
		static constexpr std::wstring_view S_CMRECT_VS_NAME = L"CMRect_VS.cso";
		static constexpr std::wstring_view S_CMRECT_PS_NAME = L"CMRect_PS.cso";
		static constexpr std::wstring_view S_CMCUBE_VS_NAME = L"CMCube_VS.cso";
		static constexpr std::wstring_view S_CMCUBE_PS_NAME = L"CMCube_PS.cso";
		static constexpr std::wstring_view S_CMCIRCLE_VS_NAME = L"CMCircle_VS.cso";
		static constexpr std::wstring_view S_CMCIRCLE_PS_NAME = L"CMCircle_PS.cso";
		CMCommon::CMLoggerWide& m_CMLoggerRef;
		std::wstring m_CompiledShaderDirectory;
		std::vector<std::shared_ptr<IDXShaderSet>> m_ShaderSets;
		bool m_Initialized = false;
		bool m_Shutdown = false;
	};
}