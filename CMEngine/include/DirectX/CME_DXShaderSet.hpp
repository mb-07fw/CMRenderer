#pragma once

#include <type_traits>

#include "DirectX/CME_DXShaderData.hpp"
#include "DirectX/CME_DXComponents.hpp"
#include "CMC_Logger.hpp"

namespace CMEngine::DirectXAPI::DX11
{
	struct IDXShaderSet
	{
		IDXShaderSet(DXShaderSetType type, const DXShaderData& vertexData, const DXShaderData& pixelData) noexcept;
		virtual ~IDXShaderSet() = default;

		virtual void CreateShaderSet(DXDevice& device, CMCommon::CMLoggerWide& logger) noexcept = 0;
		static void GetInputElementDescsOfType(std::vector<D3D11_INPUT_ELEMENT_DESC>& outInputElementDescs, DXShaderSetType setType) noexcept;
	protected:
		void CreateMandatoryShaders(DXDevice& device, CMCommon::CMLoggerWide& logger) noexcept;

		template <typename DerivedTy>
			requires std::is_base_of_v<IDXShaderSet, DerivedTy>
		void CreateInputLayout(DXDevice& device, CMCommon::CMLoggerWide& logger) noexcept;
	public:
		DXShaderSetType Type = DXShaderSetType::INVALID;
		DXShaderData VertexData = {};
		DXShaderData PixelData = {};
		Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout;
		bool CreatedMandatoryShaders = false;
		bool IsCreated = false;
	};

	template <typename DerivedTy>
		requires std::is_base_of_v<IDXShaderSet, DerivedTy>
	void IDXShaderSet::CreateInputLayout(DXDevice& device, CMCommon::CMLoggerWide& logger) noexcept
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> inputElementDescs;
		DerivedTy::GetInputElementDescs(inputElementDescs);

		logger.LogFatalNLIf(
			inputElementDescs.empty(),
			L"IDXShaderSet [CreateInputLayout] | Input element descriptor vector size is 0."
		);

		HRESULT hResult = device->CreateInputLayout(
			inputElementDescs.data(),
			static_cast<UINT>(inputElementDescs.size()),
			VertexData.pBytecode->GetBufferPointer(),
			VertexData.pBytecode->GetBufferSize(),
			pInputLayout.GetAddressOf()
		);

		logger.LogFatalNLIf(
			FAILED(hResult),
			L"IDXShaderSet [CreateInputLayout] | Failed to create input layout."
		);
	}

	struct DXShaderSetQuad : public IDXShaderSet
	{
		DXShaderSetQuad(const DXShaderData& vertexData, const DXShaderData& pixelData) noexcept;
		~DXShaderSetQuad() = default;

		virtual void CreateShaderSet(DXDevice& device, CMCommon::CMLoggerWide& logger) noexcept override;
		static void GetInputElementDescs(std::vector<D3D11_INPUT_ELEMENT_DESC>& outInputElementDescs) noexcept;

		static constexpr size_t S_TOTAL_INPUT_LAYOUT_DESC_ELEMENTS = 1;
	};

	struct DXShaderSetQuadDepth : public IDXShaderSet
	{
		DXShaderSetQuadDepth(const DXShaderData& vertexData, const DXShaderData& pixelData) noexcept;
		~DXShaderSetQuadDepth() = default;

		virtual void CreateShaderSet(DXDevice& device, CMCommon::CMLoggerWide& logger) noexcept override;
		static void GetInputElementDescs(std::vector<D3D11_INPUT_ELEMENT_DESC>& outInputElementDescs) noexcept;

		static constexpr size_t S_TOTAL_INPUT_LAYOUT_DESC_ELEMENTS = 1;
	};

	struct DXShaderSetCircle : public IDXShaderSet
	{
		DXShaderSetCircle(const DXShaderData& vertexData, const DXShaderData& pixelData) noexcept;
		~DXShaderSetCircle() = default;

		virtual void CreateShaderSet(DXDevice& device, CMCommon::CMLoggerWide& logger) noexcept override;
		static void GetInputElementDescs(std::vector<D3D11_INPUT_ELEMENT_DESC>& outInputElementDescs) noexcept;

		static constexpr size_t S_TOTAL_INPUT_LAYOUT_DESC_ELEMENTS = 2u;
	};
}