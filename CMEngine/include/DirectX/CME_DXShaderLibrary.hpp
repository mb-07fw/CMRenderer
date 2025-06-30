#pragma once

#include <d3d11.h>

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>
#include <type_traits>

#include "DirectX/CME_DXShaderSet.hpp"
#include "CMC_Logger.hpp"

namespace CMEngine::DirectXAPI::DX11
{
	class DXShaderLibrary
	{
	public:
		DXShaderLibrary(CMCommon::CMLoggerWide& logger) noexcept;
		~DXShaderLibrary() noexcept;
	public:
		void Init(DXDevice& device) noexcept;
		void Shutdown() noexcept;

		[[nodiscard]] std::weak_ptr<IDXShaderSet> GetSetOfType(DXShaderSetType setType) noexcept;

		inline [[nodiscard]] bool IsInitialized() const noexcept { return m_Initialized; }
		inline [[nodiscard]] bool IsShutdown() const noexcept { return m_Shutdown; }
	private:
		void LoadShaders() noexcept;

		void CreateShaderSets(DXDevice& device) noexcept;

		void GetCompiledShaderDirectory() noexcept;
		void GetAllShaderData(std::vector<DXShaderData>& outData, const std::filesystem::path& compiledShaderPath) noexcept;

		[[nodiscard]] size_t TotalCompiledShaders(const std::filesystem::path& compiledShaderPath) const noexcept;
		[[nodiscard]] DXShaderSetType CorrespondingSetType(const std::wstring& fileName) const noexcept;
	private:
		static constexpr size_t S_EXPECTED_NUM_SHADERS = 8;
		static constexpr size_t S_EXPECTED_NUM_SHADER_SETS = 4;
		static constexpr std::wstring_view S_CMQUAD_VS_NAME = L"CMQuad_VS.cso";
		static constexpr std::wstring_view S_CMQUAD_PS_NAME = L"CMQuad_PS.cso";
		static constexpr std::wstring_view S_CMQUAD_OUTLINED_VS_NAME = L"CMQuadOutlined_VS.cso";
		static constexpr std::wstring_view S_CMQUAD_OUTLINED_PS_NAME = L"CMQuadOutlined_PS.cso";
		static constexpr std::wstring_view S_CMQUAD_DEPTH_VS_NAME = L"CMQuadDepth_VS.cso";
		static constexpr std::wstring_view S_CMQUAD_DEPTH_PS_NAME = L"CMQuadDepth_PS.cso";
		static constexpr std::wstring_view S_CMCIRCLE_VS_NAME = L"CMCircle_VS.cso";
		static constexpr std::wstring_view S_CMCIRCLE_PS_NAME = L"CMCircle_PS.cso";
		CMCommon::CMLoggerWide& m_Logger;
		std::filesystem::path m_CompiledShaderPath;
		std::vector<std::shared_ptr<IDXShaderSet>> m_ShaderSets;
		bool m_Initialized = false;
		bool m_Shutdown = false;
	};
}