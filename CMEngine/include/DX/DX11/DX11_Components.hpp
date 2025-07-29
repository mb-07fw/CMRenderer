#pragma once

#include <minwindef.h>

#include <d3d11.h>
#include <d3d11_1.h>
#include <d2d1.h>
#include <dwrite.h>

#include <dxgi.h>
#include <wrl/client.h>

#include "Common/Logger.hpp"

namespace CMEngine::DX::DX11
{
	class Device
	{
	public:
		Device(Common::LoggerWide& logger) noexcept;
		~Device() noexcept;
	public:
		void Create() noexcept;
		void Release() noexcept;

		ID3D11Device1* operator->() noexcept;

		inline [[nodiscard]] ID3D11Device1* GetRaw() noexcept { return mP_Device.Get(); }
		inline [[nodiscard]] ID3D11DeviceContext1* ContextRaw() noexcept { return mP_Context.Get(); }

		inline [[nodiscard]] Microsoft::WRL::ComPtr<ID3D11Device1> Get() noexcept { return mP_Device; }
		inline [[nodiscard]] Microsoft::WRL::ComPtr<ID3D11DeviceContext1> Context() noexcept { return mP_Context; }

		inline [[nodiscard]] bool IsCreated() const noexcept { return m_Created; }
		inline [[nodiscard]] bool IsReleased() const noexcept { return m_Released; }
	private:
		void CreateDevice() noexcept;
	private:
		Microsoft::WRL::ComPtr<ID3D11Device1> mP_Device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext1> mP_Context;
		Common::LoggerWide& m_Logger;
		bool m_Created = false;
		bool m_Released = false;
	};
	
	class Factory
	{
	public:
		Factory(Common::LoggerWide& logger) noexcept;
		~Factory() noexcept;
	public:
		void Create(Device& device) noexcept;
		void Release() noexcept;

		IDXGIFactory1* operator->() noexcept;

		inline [[nodiscard]] Microsoft::WRL::ComPtr<IDXGIFactory1> Get() noexcept { return mP_Factory; }
		inline [[nodiscard]] IDXGIFactory1* GetRaw() noexcept { return mP_Factory.Get(); }

		inline [[nodiscard]] bool IsCreated() const noexcept { return m_Created; }
		inline [[nodiscard]] bool IsReleased() const noexcept { return m_Released; }
	private:
		Microsoft::WRL::ComPtr<IDXGIFactory1> mP_Factory;
		Common::LoggerWide& m_Logger;
		bool m_Created = false;
		bool m_Released = false;
	};

	class SwapChain
	{
	public:
		SwapChain(Common::LoggerWide& logger) noexcept;
		~SwapChain() noexcept;
	public:
		void Create(const HWND hWnd, const RECT clientArea, Factory& factory, Device& device) noexcept;
		void Release() noexcept;

		IDXGISwapChain* operator->() noexcept;

		inline [[nodiscard]] Microsoft::WRL::ComPtr<IDXGISwapChain> Get() noexcept { return mP_SwapChain; }
		inline [[nodiscard]] IDXGISwapChain* GetRaw() noexcept { return mP_SwapChain.Get(); }

		inline [[nodiscard]] bool IsCreated() const noexcept { return m_Created; }
		inline [[nodiscard]] bool IsReleased() const noexcept { return m_Released; }
	private:
		DXGI_SWAP_CHAIN_DESC m_Desc = {};
		Microsoft::WRL::ComPtr<IDXGISwapChain> mP_SwapChain;
		Common::LoggerWide& m_Logger;
		bool m_Created = false;
		bool m_Released = false;
	};

	class InfoQueue
	{
	public:
		InfoQueue(Common::LoggerWide& logger) noexcept;
		~InfoQueue() noexcept;
	public:
		void Create(Device& device) noexcept;
		void Release() noexcept;

		void LogMessages() noexcept;
		void GetMessages(std::vector<std::wstring>& outMessages) noexcept;

		[[nodiscard]] bool IsQueueEmpty() noexcept;

		inline [[nodiscard]] bool IsCreated() const noexcept { return m_Created; }
		inline [[nodiscard]] bool IsReleased() const noexcept { return m_Released; }
	private:
		Microsoft::WRL::ComPtr<ID3D11InfoQueue> mP_InfoQueue;
		Common::LoggerWide& m_Logger;
		bool m_Created = false;
		bool m_Released = false;
	};

	/*class Writer
	{
	public:
		Writer(Common::LoggerWide& logger) noexcept;
		~Writer() noexcept;
	public:
		void Create(SwapChain& swapChain) noexcept;
		void Release() noexcept;

		void WriteText(std::wstring_view text, float layoutLeft, float layoutTop, float layoutRight, float layoutBottom) noexcept;

		inline [[nodiscard]] bool IsCreated() const noexcept { return m_Created; }
		inline [[nodiscard]] bool IsReleased() const noexcept { return m_Released; }
	private:
		void CreateIndependentResources() noexcept;
		void CreateDependentResources(SwapChain& swapChain) noexcept;
	private:
		Microsoft::WRL::ComPtr<ID2D1Factory> mP_D2DFactory;
		Microsoft::WRL::ComPtr<ID2D1RenderTarget> mP_D2DRenderTarget;
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> mP_Brush;
		Microsoft::WRL::ComPtr<IDWriteFactory> mP_DWriteFactory;
		Microsoft::WRL::ComPtr<IDWriteTextFormat> mP_TextFormat;
		Common::LoggerWide& m_Logger;
		bool m_Created = false;
		bool m_Released = false;
	};*/
}