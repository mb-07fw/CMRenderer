#pragma once

#include "CMPlatform/IGraphics.hpp"
#include "CMPlatform/WinImpl_Window.hpp"
#include "CMPlatform/PlatformTypes.hpp"
#include "CMPlatform/Export.hpp"


#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_5.h>
#include <wrl/client.h>

#include <array>
#include <vector>
#include <string_view>

#include <cstdint>

namespace CMEngine::Platform::WinImpl
{
	inline constexpr ShaderSetEnum::Enum ActiveToCorrespondingType(ActiveShaderEnum::Enum activeType) noexcept
	{
		switch (activeType)
		{
		case ActiveShaderEnum::QUAD_VS: [[fallthrough]];
		case ActiveShaderEnum::QUAD_PS:
			return ShaderSetEnum::QUAD;
		case ActiveShaderEnum::INVALID: [[fallthrough]];
		default:
			return ShaderSetEnum::INVALID;
		}
	}

	struct ShaderData
	{
		ShaderEnum::Enum Type = ShaderEnum::Enum::INVALID;
		ActiveShaderEnum::Enum ActiveType = ActiveShaderEnum::Enum::INVALID;
		ShaderSetEnum::Enum CorrespondingSet = ShaderSetEnum::Enum::INVALID;
		std::wstring Name;
		Microsoft::WRL::ComPtr<ID3DBlob> pBytecode;

		inline ShaderData(
			ShaderEnum::Enum type,
			ActiveShaderEnum::Enum activeType,
			ShaderSetEnum::Enum correspondingSet,
			const std::wstring& name,
			Microsoft::WRL::ComPtr<ID3DBlob> pBytecode
		) noexcept
			: Type(type),
			  ActiveType(activeType),
			  CorrespondingSet(correspondingSet),
			  Name(name),
			  pBytecode(pBytecode)
		{
		}

		~ShaderData() = default;
	};

	struct BasicShaderSet
	{
		ShaderData VertexData;
		ShaderData PixelData;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader;

		inline BasicShaderSet(const ShaderData& vertexData, const ShaderData& pixelData) noexcept
			: VertexData(vertexData),
			  PixelData(pixelData)
		{
		}

		virtual void CreateShaders(Microsoft::WRL::ComPtr<ID3D11Device> pDevice) noexcept = 0;
	protected:
		void CreateBasicShaders(Microsoft::WRL::ComPtr<ID3D11Device> pDevice) noexcept;
	};

	struct ShaderSetQuad : public BasicShaderSet
	{
		using BasicShaderSet::BasicShaderSet;

		~ShaderSetQuad() = default;

		virtual void CreateShaders(Microsoft::WRL::ComPtr<ID3D11Device> pDevice) noexcept override;
	};
	
	class ShaderLibrary
	{
	public:
		ShaderLibrary() noexcept;
		~ShaderLibrary() noexcept;
	public:
		void CreateResources(Microsoft::WRL::ComPtr<ID3D11Device> pDevice) noexcept;
	private:
		void Init() noexcept;
		void Shutdown() noexcept;

		void LoadAll() noexcept;

		void LoadShaders(std::vector<ShaderData>& outData) noexcept;
		void CreateShaderSets(const std::vector<ShaderData>& data) noexcept;
	private:
		static constexpr std::wstring_view S_VERTEX_SHADER_FLAG = L"VS";
		static constexpr std::wstring_view S_PIXEL_SHADER_FLAG = L"PS";

		std::unordered_map<std::wstring, ActiveShaderEnum::Enum> m_ShaderNameMap = {
			{ { L"Quad_VS.cso" }, ActiveShaderEnum::QUAD_VS },
			{ { L"Quad_PS.cso" }, ActiveShaderEnum::QUAD_PS }
		};

		std::vector<std::shared_ptr<BasicShaderSet>> m_Sets;
	};

	class Graphics : public IGraphics
	{
	public:
		Graphics(Window& window) noexcept;
		~Graphics() noexcept;

		Graphics(const Graphics& other) = delete;
		Graphics& operator=(const Graphics& other) = delete;
	public:
		void Impl_Update() noexcept;
		void Impl_Clear(ColorNorm color) noexcept;
		void Impl_Present() noexcept;
	private:
		void Impl_Init() noexcept;
		void Impl_Shutdown() noexcept;

		void Impl_InitPipeline() noexcept;

		/* (Views in this context refer to RTV's and DSV's on the swap chain) */
		void Impl_CreateViews() noexcept;
		void Impl_ReleaseViews() noexcept;

		void Impl_BindViews() noexcept;

		void Impl_OnResizeCallback(ScreenResolution resolution) noexcept;

		static void Impl_OnResizeThunk(ScreenResolution resolution, void* pThis) noexcept;
	private:
		ShaderLibrary m_Library;
		Window& m_Window; /* TODO: Come up with a better solution for this... */
		Microsoft::WRL::ComPtr<ID3D11Device> mP_Device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> mP_Context;
		Microsoft::WRL::ComPtr<IDXGIFactory5> mP_Factory;
		Microsoft::WRL::ComPtr<IDXGISwapChain1> mP_SwapChain;
		Microsoft::WRL::ComPtr<ID3D11InfoQueue> mP_InfoQueue;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mP_RTV;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mP_DSV;
		static constexpr UINT S_PRESENT_SYNC_INTERVAL_TEARING = 0;
		static constexpr UINT S_PRESENT_SYNC_INTERVAL_VSYNC = 1;
		UINT m_PresentSyncInterval = S_PRESENT_SYNC_INTERVAL_VSYNC;
		UINT m_PresentFlags = 0;
	};

	CM_DYNAMIC_LOAD void WinImpl_Graphics_Clear(ColorNorm color);
	CM_DYNAMIC_LOAD void WinImpl_Graphics_Present();
}