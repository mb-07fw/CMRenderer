#pragma once

#include <d3d11.h>

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>
#include <type_traits>

#include "DX/DX11/DX11_ShaderSet.hpp"
#include "Common/Logger.hpp"
#include "Common/FileWatcher.hpp"

namespace CMEngine::DX::DX11
{
	class ShaderLibrary
	{
	public:
		ShaderLibrary(Common::LoggerWide& logger) noexcept;
		~ShaderLibrary() noexcept;
	public:
		void Init(Device& device) noexcept;
		void Shutdown() noexcept;

		[[nodiscard]] std::weak_ptr<IShaderSet> GetSetOfType(ShaderSetType setType) noexcept;

		inline [[nodiscard]] bool IsInitialized() const noexcept { return m_Initialized; }
		inline [[nodiscard]] bool IsShutdown() const noexcept { return m_Shutdown; }
	private:
		void LoadShaders() noexcept;

		void CreateShaderSets(Device& device) noexcept;

		void GetCompiledShaderDirectory() noexcept;
		void GetAllShaderData(std::vector<ShaderData>& outData, const std::filesystem::path& compiledShaderPath) noexcept;

		[[nodiscard]] size_t TotalCompiledShaders(const std::filesystem::path& compiledShaderPath) const noexcept;
		[[nodiscard]] ShaderSetType CorrespondingSetType(const std::wstring& fileName) const noexcept;
	private:
		static constexpr size_t S_EXPECTED_NUM_SHADERS = 8;
		static constexpr size_t S_EXPECTED_NUM_SHADER_SETS = 4;
		/* TODO: Use a dictionary instead of if-else's and turn already known names into keys.*/
		static constexpr std::wstring_view S_QUAD_VS_NAME = L"Quad_VS.cso";
		static constexpr std::wstring_view S_QUAD_PS_NAME = L"Quad_PS.cso";
		static constexpr std::wstring_view S_QUAD_OUTLINED_VS_NAME = L"QuadOutlined_VS.cso";
		static constexpr std::wstring_view S_QUAD_OUTLINED_PS_NAME = L"QuadOutlined_PS.cso";
		static constexpr std::wstring_view S_QUAD_DEPTH_VS_NAME = L"QuadDepth_VS.cso";
		static constexpr std::wstring_view S_QUAD_DEPTH_PS_NAME = L"QuadDepth_PS.cso";
		static constexpr std::wstring_view S_CIRCLE_VS_NAME = L"Circle_VS.cso";
		static constexpr std::wstring_view S_CIRCLE_PS_NAME = L"Circle_PS.cso";
		Common::LoggerWide& m_Logger;
		std::filesystem::path m_CompiledShaderPath;
		std::vector<std::shared_ptr<IShaderSet>> m_ShaderSets;
		Common::FileWatcher m_ShaderWatcher;
		bool m_Initialized = false;
		bool m_Shutdown = false;
	};
}