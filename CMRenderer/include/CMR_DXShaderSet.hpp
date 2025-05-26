#pragma once

#include <type_traits>

#include "CMR_DXShaderData.hpp"
#include "CMC_Logger.hpp"
#include "CMR_DXComponents.hpp"

namespace CMRenderer::CMDirectX
{
	struct IDXShaderSet
	{
		IDXShaderSet(DXShaderSetType type, const DXShaderData& vertexDataRef, const DXShaderData& pixelDataRef) noexcept;
		virtual ~IDXShaderSet() = default;

		virtual void CreateShaderSet(Components::DXDevice& deviceRef, CMCommon::CMLoggerWide& cmLoggerRef) noexcept = 0;
		static void GetInputElementDescsOfType(std::vector<D3D11_INPUT_ELEMENT_DESC>& outInputElementDescs, DXShaderSetType setType) noexcept;
	protected:
		void CreateMandatoryShaders(Components::DXDevice& deviceRef, CMCommon::CMLoggerWide& cmLoggerRef) noexcept;

		template <typename DerivedTy>
			requires std::is_base_of_v<IDXShaderSet, DerivedTy>
		void CreateInputLayout(Components::DXDevice& deviceRef, CMCommon::CMLoggerWide& cmLoggerRef) noexcept;
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
	void IDXShaderSet::CreateInputLayout(Components::DXDevice& deviceRef, CMCommon::CMLoggerWide& cmLoggerRef) noexcept
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> inputElementDescs;
		DerivedTy::GetInputElementDescs(inputElementDescs);

		cmLoggerRef.LogFatalNLIf(inputElementDescs.size() == 0, L"IDXShaderSet [CreateInputLayout] | Input element descriptor vector size is 0.");

		HRESULT hResult = deviceRef->CreateInputLayout(
			inputElementDescs.data(),
			static_cast<UINT>(inputElementDescs.size()),
			VertexData.pBytecode->GetBufferPointer(),
			VertexData.pBytecode->GetBufferSize(),
			pInputLayout.GetAddressOf()
		);

		cmLoggerRef.LogFatalNLIf(FAILED(hResult), L"IDXShaderSet [CreateInputLayout] | Failed to create input layout.");
	}

	struct DXShaderSetQuad : public IDXShaderSet
	{
		DXShaderSetQuad(const DXShaderData& vertexDataRef, const DXShaderData& pixelDataRef) noexcept;
		~DXShaderSetQuad() = default;

		virtual void CreateShaderSet(Components::DXDevice& deviceRef, CMCommon::CMLoggerWide& cmLoggerRef) noexcept override;
		static void GetInputElementDescs(std::vector<D3D11_INPUT_ELEMENT_DESC>& outInputElementDescs) noexcept;

		static constexpr size_t S_TOTAL_INPUT_LAYOUT_DESC_ELEMENTS = 1;
	};

	struct DXShaderSetQuadDepth : public IDXShaderSet
	{
		DXShaderSetQuadDepth(const DXShaderData& vertexDataRef, const DXShaderData& pixelDataRef) noexcept;
		~DXShaderSetQuadDepth() = default;

		virtual void CreateShaderSet(Components::DXDevice& deviceRef, CMCommon::CMLoggerWide& cmLoggerRef) noexcept override;
		static void GetInputElementDescs(std::vector<D3D11_INPUT_ELEMENT_DESC>& outInputElementDescs) noexcept;

		static constexpr size_t S_TOTAL_INPUT_LAYOUT_DESC_ELEMENTS = 1;
	};

	struct DXShaderSetCircle : public IDXShaderSet
	{
		DXShaderSetCircle(const DXShaderData& vertexDataRef, const DXShaderData& pixelDataRef) noexcept;
		~DXShaderSetCircle() = default;

		virtual void CreateShaderSet(Components::DXDevice& deviceRef, CMCommon::CMLoggerWide& cmLoggerRef) noexcept override;
		static void GetInputElementDescs(std::vector<D3D11_INPUT_ELEMENT_DESC>& outInputElementDescs) noexcept;

		static constexpr size_t S_TOTAL_INPUT_LAYOUT_DESC_ELEMENTS = 2u;
	};
}