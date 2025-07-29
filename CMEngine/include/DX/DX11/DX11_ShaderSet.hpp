#pragma once

#include <type_traits>

#include "DX/DX11/DX11_ShaderData.hpp"
#include "DX/DX11/DX11_Components.hpp"
#include "Common/Logger.hpp"

namespace CMEngine::DX::DX11
{
	struct IShaderSet
	{
		IShaderSet(ShaderSetType type, const ShaderData& vertexData, const ShaderData& pixelData) noexcept;
		virtual ~IShaderSet() = default;

		virtual void CreateShaderSet(Device& device, Common::LoggerWide& logger) noexcept = 0;
		static void GetInputElementDescsOfType(std::vector<D3D11_INPUT_ELEMENT_DESC>& outInputElementDescs, ShaderSetType setType) noexcept;
	protected:
		void CreateMandatoryShaders(Device& device, Common::LoggerWide& logger) noexcept;

		template <typename DerivedTy>
			requires std::is_base_of_v<IShaderSet, DerivedTy>
		void CreateInputLayout(Device& device, Common::LoggerWide& logger) noexcept;
	public:
		ShaderSetType Type = ShaderSetType::INVALID;
		ShaderData VertexData = {};
		ShaderData PixelData = {};
		Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout;
		bool CreatedMandatoryShaders = false;
		bool IsCreated = false;
	};

	template <typename DerivedTy>
		requires std::is_base_of_v<IShaderSet, DerivedTy>
	void IShaderSet::CreateInputLayout(Device& device, Common::LoggerWide& logger) noexcept
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> inputElementDescs;
		DerivedTy::GetInputElementDescs(inputElementDescs);

		logger.LogFatalNLIf(
			inputElementDescs.empty(),
			L"IShaderSet [CreateInputLayout] | Input element descriptor vector size is 0."
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
			L"IShaderSet [CreateInputLayout] | Failed to create input layout."
		);
	}

	struct ShaderSetQuad : public IShaderSet
	{
		ShaderSetQuad(const ShaderData& vertexData, const ShaderData& pixelData) noexcept;
		~ShaderSetQuad() = default;

		virtual void CreateShaderSet(Device& device, Common::LoggerWide& logger) noexcept override;
		static void GetInputElementDescs(std::vector<D3D11_INPUT_ELEMENT_DESC>& outInputElementDescs) noexcept;

		static constexpr size_t S_TOTAL_INPUT_LAYOUT_DESC_ELEMENTS = 1;
	};

	struct ShaderSetQuadOutlined : public IShaderSet
	{
		ShaderSetQuadOutlined(const ShaderData& vertexData, const ShaderData& pixelData) noexcept;
		~ShaderSetQuadOutlined() = default;

		virtual void CreateShaderSet(Device& device, Common::LoggerWide& logger) noexcept override;
		static void GetInputElementDescs(std::vector<D3D11_INPUT_ELEMENT_DESC>& outInputElementDescs) noexcept;

		static constexpr size_t S_TOTAL_INPUT_LAYOUT_DESC_ELEMENTS = 1;
	};

	struct ShaderSetQuadDepth : public IShaderSet
	{
		ShaderSetQuadDepth(const ShaderData& vertexData, const ShaderData& pixelData) noexcept;
		~ShaderSetQuadDepth() = default;

		virtual void CreateShaderSet(Device& device, Common::LoggerWide& logger) noexcept override;
		static void GetInputElementDescs(std::vector<D3D11_INPUT_ELEMENT_DESC>& outInputElementDescs) noexcept;

		static constexpr size_t S_TOTAL_INPUT_LAYOUT_DESC_ELEMENTS = 1;
	};

	struct ShaderSetCircle : public IShaderSet
	{
		ShaderSetCircle(const ShaderData& vertexData, const ShaderData& pixelData) noexcept;
		~ShaderSetCircle() = default;

		virtual void CreateShaderSet(Device& device, Common::LoggerWide& logger) noexcept override;
		static void GetInputElementDescs(std::vector<D3D11_INPUT_ELEMENT_DESC>& outInputElementDescs) noexcept;

		static constexpr size_t S_TOTAL_INPUT_LAYOUT_DESC_ELEMENTS = 2u;
	};
}