#pragma once

#include <d3d11.h>

#include <dxgi.h>
#include <Windows.h>
#include <wrl/client.h>

#include "Internal/Utility/CMLogger.hpp"

namespace CMRenderer::CMDirectX::Components
{
	class DXDevice
	{
	public:
		DXDevice(Utility::CMLoggerWide& cmLoggerRef) noexcept;
		~DXDevice() noexcept;
	public:
		void Create() noexcept;
		void Release() noexcept;

		ID3D11Device* operator->() noexcept;

		inline [[nodiscard]] ID3D11Device* DeviceRaw() noexcept { return mP_Device.Get(); }
		inline [[nodiscard]] ID3D11DeviceContext* ContextRaw() noexcept { return mP_Context.Get(); }

		inline [[nodiscard]] Microsoft::WRL::ComPtr<ID3D11Device> Device() noexcept { return mP_Device; }
		inline [[nodiscard]] Microsoft::WRL::ComPtr<ID3D11DeviceContext> Context() noexcept { return mP_Context; }

		inline [[nodiscard]] bool IsCreated() const noexcept { return m_Created; }
		inline [[nodiscard]] bool IsReleased() const noexcept { return m_Released; }
	private:
		void CreateDevice() noexcept;
	private:
		Microsoft::WRL::ComPtr<ID3D11Device> mP_Device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> mP_Context;
		Utility::CMLoggerWide& m_CMLoggerRef;
		bool m_Created = false;
		bool m_Released = false;
	};


	
	class DXFactory
	{
	public:
		DXFactory(Utility::CMLoggerWide& cmLoggerRef) noexcept;
		~DXFactory() noexcept;
	public:
		void Create(DXDevice& deviceRef) noexcept;
		void Release() noexcept;

		IDXGIFactory1* operator->() noexcept;

		inline [[nodiscard]] Microsoft::WRL::ComPtr<IDXGIFactory1> Factory() noexcept { return mP_Factory; }
		inline [[nodiscard]] IDXGIFactory1* FactoryRaw() noexcept { return mP_Factory.Get(); }

		inline [[nodiscard]] bool IsCreated() const noexcept { return m_Created; }
		inline [[nodiscard]] bool IsReleased() const noexcept { return m_Released; }
	private:
		Microsoft::WRL::ComPtr<IDXGIFactory1> mP_Factory;
		Utility::CMLoggerWide& m_CMLoggerRef;
		bool m_Created = false;
		bool m_Released = false;
	};



	class DXSwapChain
	{
	public:
		DXSwapChain(Utility::CMLoggerWide& cmLoggerRef) noexcept;
		~DXSwapChain() noexcept;
	public:
		void Create(const HWND hWnd, const RECT clientArea, DXFactory& factoryRef, DXDevice& deviceRef) noexcept;
		void Release() noexcept;

		IDXGISwapChain* operator->() noexcept;

		inline [[nodiscard]] Microsoft::WRL::ComPtr<IDXGISwapChain> SwapChain() noexcept { return mP_SwapChain; }
		inline [[nodiscard]] IDXGISwapChain* SwapChainRaw() noexcept { return mP_SwapChain.Get(); }

		inline [[nodiscard]] bool IsCreated() const noexcept { return m_Created; }
		inline [[nodiscard]] bool IsReleased() const noexcept { return m_Released; }
	private:
		DXGI_SWAP_CHAIN_DESC m_Desc = {};
		Microsoft::WRL::ComPtr<IDXGISwapChain> mP_SwapChain;
		Utility::CMLoggerWide& m_CMLoggerRef;
		bool m_Created = false;
		bool m_Released = false;
	};



	class DXInfoQueue
	{
	public:
		DXInfoQueue(Utility::CMLoggerWide& cmLoggerRef) noexcept;
		~DXInfoQueue() noexcept;
	public:
		void Create(DXDevice& deviceRef) noexcept;
		void Release() noexcept;

		void LogMessages() noexcept;
		void GetMessages(std::vector<std::wstring>& outMessages) noexcept;

		[[nodiscard]] bool IsQueueEmpty() noexcept;

		inline [[nodiscard]] bool IsCreated() const noexcept { return m_Created; }
		inline [[nodiscard]] bool IsReleased() const noexcept { return m_Released; }
	private:
		Microsoft::WRL::ComPtr<ID3D11InfoQueue> mP_InfoQueue;
		Utility::CMLoggerWide& m_CMLoggerRef;
		bool m_Created = false;
		bool m_Released = false;
	};



	class DXWriter
	{
	public:
		DXWriter(Utility::CMLoggerWide& cmLoggerRef) noexcept;
		~DXWriter() noexcept;
	public:
		void Create(DXSwapChain& swapChainRef) noexcept;
		void Release() noexcept;

		void WriteText(std::wstring_view text) noexcept;

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
		Utility::CMLoggerWide& m_CMLoggerRef;
		bool m_Created = false;
		bool m_Released = false;
	};
}