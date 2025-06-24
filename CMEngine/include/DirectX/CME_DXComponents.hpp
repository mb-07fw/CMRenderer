#pragma once

#include <minwindef.h>

#include <d3d11.h>
#include <d3d11_1.h>
#include <d2d1.h>
#include <dwrite.h>

#include <dxgi.h>
#include <wrl/client.h>

#include "CMC_Logger.hpp"

namespace CMEngine::DirectXAPI::DX11
{
	class DXDevice
	{
	public:
		DXDevice(CMCommon::CMLoggerWide& logger) noexcept;
		~DXDevice() noexcept;
	public:
		void Create() noexcept;
		void Release() noexcept;

		ID3D11Device1* operator->() noexcept;

		inline [[nodiscard]] ID3D11Device1* DeviceRaw() noexcept { return mP_Device.Get(); }
		inline [[nodiscard]] ID3D11DeviceContext1* ContextRaw() noexcept { return mP_Context.Get(); }

		inline [[nodiscard]] Microsoft::WRL::ComPtr<ID3D11Device1> Device() noexcept { return mP_Device; }
		inline [[nodiscard]] Microsoft::WRL::ComPtr<ID3D11DeviceContext1> Context() noexcept { return mP_Context; }

		inline [[nodiscard]] bool IsCreated() const noexcept { return m_Created; }
		inline [[nodiscard]] bool IsReleased() const noexcept { return m_Released; }
	private:
		void CreateDevice() noexcept;
	private:
		Microsoft::WRL::ComPtr<ID3D11Device1> mP_Device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext1> mP_Context;
		CMCommon::CMLoggerWide& m_Logger;
		bool m_Created = false;
		bool m_Released = false;
	};
	
	class DXFactory
	{
	public:
		DXFactory(CMCommon::CMLoggerWide& logger) noexcept;
		~DXFactory() noexcept;
	public:
		void Create(DXDevice& device) noexcept;
		void Release() noexcept;

		IDXGIFactory1* operator->() noexcept;

		inline [[nodiscard]] Microsoft::WRL::ComPtr<IDXGIFactory1> Factory() noexcept { return mP_Factory; }
		inline [[nodiscard]] IDXGIFactory1* FactoryRaw() noexcept { return mP_Factory.Get(); }

		inline [[nodiscard]] bool IsCreated() const noexcept { return m_Created; }
		inline [[nodiscard]] bool IsReleased() const noexcept { return m_Released; }
	private:
		Microsoft::WRL::ComPtr<IDXGIFactory1> mP_Factory;
		CMCommon::CMLoggerWide& m_Logger;
		bool m_Created = false;
		bool m_Released = false;
	};

	class DXSwapChain
	{
	public:
		DXSwapChain(CMCommon::CMLoggerWide& logger) noexcept;
		~DXSwapChain() noexcept;
	public:
		void Create(const HWND hWnd, const RECT clientArea, DXFactory& factory, DXDevice& device) noexcept;
		void Release() noexcept;

		IDXGISwapChain* operator->() noexcept;

		inline [[nodiscard]] Microsoft::WRL::ComPtr<IDXGISwapChain> SwapChain() noexcept { return mP_SwapChain; }
		inline [[nodiscard]] IDXGISwapChain* SwapChainRaw() noexcept { return mP_SwapChain.Get(); }

		inline [[nodiscard]] bool IsCreated() const noexcept { return m_Created; }
		inline [[nodiscard]] bool IsReleased() const noexcept { return m_Released; }
	private:
		DXGI_SWAP_CHAIN_DESC m_Desc = {};
		Microsoft::WRL::ComPtr<IDXGISwapChain> mP_SwapChain;
		CMCommon::CMLoggerWide& m_Logger;
		bool m_Created = false;
		bool m_Released = false;
	};

	class DXInfoQueue
	{
	public:
		DXInfoQueue(CMCommon::CMLoggerWide& logger) noexcept;
		~DXInfoQueue() noexcept;
	public:
		void Create(DXDevice& device) noexcept;
		void Release() noexcept;

		void LogMessages() noexcept;
		void GetMessages(std::vector<std::wstring>& outMessages) noexcept;

		[[nodiscard]] bool IsQueueEmpty() noexcept;

		inline [[nodiscard]] bool IsCreated() const noexcept { return m_Created; }
		inline [[nodiscard]] bool IsReleased() const noexcept { return m_Released; }
	private:
		Microsoft::WRL::ComPtr<ID3D11InfoQueue> mP_InfoQueue;
		CMCommon::CMLoggerWide& m_Logger;
		bool m_Created = false;
		bool m_Released = false;
	};

	/*class DXWriter
	{
	public:
		DXWriter(CMCommon::CMLoggerWide& logger) noexcept;
		~DXWriter() noexcept;
	public:
		void Create(DXSwapChain& swapChain) noexcept;
		void Release() noexcept;

		void WriteText(std::wstring_view text, float layoutLeft, float layoutTop, float layoutRight, float layoutBottom) noexcept;

		inline [[nodiscard]] bool IsCreated() const noexcept { return m_Created; }
		inline [[nodiscard]] bool IsReleased() const noexcept { return m_Released; }
	private:
		void CreateIndependentResources() noexcept;
		void CreateDependentResources(DXSwapChain& swapChainRef) noexcept;
	private:
		Microsoft::WRL::ComPtr<ID2D1Factory> mP_D2DFactory;
		Microsoft::WRL::ComPtr<ID2D1RenderTarget> mP_D2DRenderTarget;
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> mP_Brush;
		Microsoft::WRL::ComPtr<IDWriteFactory> mP_DWriteFactory;
		Microsoft::WRL::ComPtr<IDWriteTextFormat> mP_TextFormat;
		CMCommon::CMLoggerWide& m_CMLoggerRef;
		bool m_Created = false;
		bool m_Released = false;
	};*/
}