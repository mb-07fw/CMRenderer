#pragma once

#include <d3d11.h>

#include <dxgi.h>
#include <Windows.h>
#include <wrl/client.h>

#include "Internal/CMLogger.hpp"

namespace CMRenderer::CMDirectX::Components
{
#pragma region DXDevice
	class DXDevice
	{
	public:
		DXDevice(CMLoggerWide& cmLoggerRef) noexcept;
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
		bool m_Created = false;
		bool m_Released = false;
		CMLoggerWide& m_CMLoggerRef;
		Microsoft::WRL::ComPtr<ID3D11Device> mP_Device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> mP_Context;
	};
#pragma endregion


	
	class DXFactory
	{
	public:
		DXFactory(CMLoggerWide& cmLoggerRef) noexcept;
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
		bool m_Created = false;
		bool m_Released = false;
		CMLoggerWide& m_CMLoggerRef;
		Microsoft::WRL::ComPtr<IDXGIFactory1> mP_Factory;
	};



	class DXSwapChain
	{
	public:
		DXSwapChain(CMLoggerWide& cmLoggerRef) noexcept;
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
		bool m_Created = false;
		bool m_Released = false;
		CMLoggerWide& m_CMLoggerRef;
		DXGI_SWAP_CHAIN_DESC m_Desc = {};
		Microsoft::WRL::ComPtr<IDXGISwapChain> mP_SwapChain;
	};



	class DXInfoQueue
	{
	public:
		DXInfoQueue(CMLoggerWide& cmLoggerRef) noexcept;
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
		bool m_Created = false;
		bool m_Released = false;
		CMLoggerWide& m_CMLoggerRef;
		Microsoft::WRL::ComPtr<ID3D11InfoQueue> mP_InfoQueue;
	};
}