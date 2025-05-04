#pragma once

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

	struct DXShaderSetCMRect : public IDXShaderSet
	{
		DXShaderSetCMRect(const DXShaderData& vertexDataRef, const DXShaderData& pixelDataRef) noexcept;
		~DXShaderSetCMRect() = default;

		virtual void CreateShaderSet(Components::DXDevice& deviceRef, CMCommon::CMLoggerWide& cmLoggerRef) noexcept override;
		static void GetInputElementDescs(std::vector<D3D11_INPUT_ELEMENT_DESC>& outInputElementDescs) noexcept;
	private:
		static constexpr size_t S_TOTAL_INPUT_LAYOUT_DESC_ELEMENTS = 1;
	};

	struct DXShaderSetCMCube : public IDXShaderSet
	{
		DXShaderSetCMCube(const DXShaderData& vertexDataRef, const DXShaderData& pixelDataRef) noexcept;
		~DXShaderSetCMCube() = default;

		virtual void CreateShaderSet(Components::DXDevice& deviceRef, CMCommon::CMLoggerWide& cmLoggerRef) noexcept override;
		static void GetInputElementDescs(std::vector<D3D11_INPUT_ELEMENT_DESC>& outInputElementDescs) noexcept;
	private:
		static constexpr size_t S_TOTAL_INPUT_LAYOUT_DESC_ELEMENTS = 1;
	};
}