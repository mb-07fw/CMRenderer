#include "CMR_PCH.hpp"
#include "CMR_DXShaderSet.hpp"

namespace CMRenderer::CMDirectX
{
	IDXShaderSet::IDXShaderSet(DXShaderSetType type, const DXShaderData& vertexDataRef, const DXShaderData& pixelDataRef) noexcept
		: Type(type), VertexData(vertexDataRef), PixelData(pixelDataRef)
	{
	}

	void IDXShaderSet::GetInputElementDescsOfType(std::vector<D3D11_INPUT_ELEMENT_DESC>& outInputElementDescs, DXShaderSetType setType) noexcept
	{
		switch (setType)
		{
		case DXShaderSetType::INVALID: return;
		case DXShaderSetType::CMRECT:
			DXShaderSetCMRect::GetInputElementDescs(outInputElementDescs);
			return;
		default: return;
		}
	}

	void IDXShaderSet::CreateMandatoryShaders(Components::DXDevice& deviceRef, CMCommon::CMLoggerWide& cmLoggerRef) noexcept
	{
		cmLoggerRef.LogFatalNLIf(!deviceRef.IsCreated(), L"IDXShaderSet [CreateMandatoryShaders] | Device isn't created.");

		cmLoggerRef.LogFatalNLIf(VertexData.pBytecode.Get() == nullptr, L"IDXShaderSet [CreateMandatoryShaders] | Vertex shader bytecode isn't present.");
		cmLoggerRef.LogFatalNLIf(PixelData.pBytecode.Get() == nullptr, L"IDXShaderSet [CreateMandatoryShaders] | Pixel shader bytecode isn't present.");

		cmLoggerRef.LogFatalNLIf(
			VertexData.CorrespondingShaderSet == DXShaderSetType::INVALID,
			L"IDXShaderSet[CreateMandatoryShaders] | Vertex data doesn't have a corresponding shader set."
		);

		cmLoggerRef.LogFatalNLIf(
			PixelData.CorrespondingShaderSet == DXShaderSetType::INVALID,
			L"IDXShaderSet[CreateMandatoryShaders] | Pixel data doesn't have a corresponding shader set."
		);

		HRESULT hResult = S_OK;

		hResult = deviceRef->CreateVertexShader(VertexData.pBytecode->GetBufferPointer(), VertexData.pBytecode->GetBufferSize(), nullptr, &pVertexShader);

		cmLoggerRef.LogFatalNLIf(hResult != S_OK, L"IDXShaderSet [CreateMandatoryShaders] | Failed to create vertex shader.");

		hResult = deviceRef->CreatePixelShader(PixelData.pBytecode->GetBufferPointer(), PixelData.pBytecode->GetBufferSize(), nullptr, &pPixelShader);

		cmLoggerRef.LogFatalNLIf(hResult != S_OK, L"IDXShaderSet [CreateMandatoryShaders] | Failed to create pixel shader.");

		CreatedMandatoryShaders = true;
	}

	DXShaderSetCMRect::DXShaderSetCMRect(const DXShaderData& vertexDataRef, const DXShaderData& pixelDataRef) noexcept
		: IDXShaderSet(DXShaderSetType::CMRECT, vertexDataRef, pixelDataRef)
	{
	}

	void DXShaderSetCMRect::CreateShaderSet(Components::DXDevice& deviceRef, CMCommon::CMLoggerWide& cmLoggerRef) noexcept
	{
		CreateMandatoryShaders(deviceRef, cmLoggerRef);

		std::vector<D3D11_INPUT_ELEMENT_DESC> inputElementDescs;
		GetInputElementDescs(inputElementDescs);

		cmLoggerRef.LogFatalNLIf(inputElementDescs.size() == 0, L"DXShaderSetCMRect [CreateShaderSet] | Input element descriptor vector size is 0.");

		HRESULT hResult = deviceRef->CreateInputLayout(
			inputElementDescs.data(),
			static_cast<UINT>(inputElementDescs.size()),
			VertexData.pBytecode->GetBufferPointer(),
			VertexData.pBytecode->GetBufferSize(),
			pInputLayout.GetAddressOf()
		);

		cmLoggerRef.LogFatalNLIf(hResult != S_OK, L"DXShaderSetCMRect [CreateShaderSet] | Failed to create input layout.");

		cmLoggerRef.LogInfoNL(L"DXShaderSetCMRect [CreateShaderSet] | Created shader set.");

		IsCreated = true;
	}

	void DXShaderSetCMRect::GetInputElementDescs(std::vector<D3D11_INPUT_ELEMENT_DESC>& outInputElementDescs) noexcept
	{
		outInputElementDescs.clear();
		outInputElementDescs.reserve(S_TOTAL_INPUT_LAYOUT_DESC_ELEMENTS);

		D3D11_INPUT_ELEMENT_DESC descs[] = {
			{ "VertexPos2D", 0u, DXGI_FORMAT_R32G32_FLOAT, 0u, 0u, D3D11_INPUT_PER_VERTEX_DATA, 0u }
		};

		outInputElementDescs.assign(std::begin(descs), std::end(descs));
	}

	DXShaderSetCMCube::DXShaderSetCMCube(const DXShaderData& vertexDataRef, const DXShaderData& pixelDataRef) noexcept
		: IDXShaderSet(DXShaderSetType::CMCUBE, vertexDataRef, pixelDataRef)
	{
	}

	void DXShaderSetCMCube::CreateShaderSet(Components::DXDevice& deviceRef, CMCommon::CMLoggerWide& cmLoggerRef) noexcept
	{
		CreateMandatoryShaders(deviceRef, cmLoggerRef);

		std::vector<D3D11_INPUT_ELEMENT_DESC> inputElementDescs;
		GetInputElementDescs(inputElementDescs);

		cmLoggerRef.LogFatalNLIf(inputElementDescs.size() == 0, L"DXShaderSetCMCube [CreateShaderSet] | Input element descriptor vector size is 0.");

		HRESULT hResult = deviceRef->CreateInputLayout(
			inputElementDescs.data(),
			static_cast<UINT>(inputElementDescs.size()),
			VertexData.pBytecode->GetBufferPointer(),
			VertexData.pBytecode->GetBufferSize(),
			pInputLayout.GetAddressOf()
		);

		cmLoggerRef.LogFatalNLIf(hResult != S_OK, L"DXShaderSetCMCube [CreateShaderSet] | Failed to create input layout.");

		cmLoggerRef.LogInfoNL(L"DXShaderSetCMCube [CreateShaderSet] | Created shader set.");

		IsCreated = true;
	}

	void DXShaderSetCMCube::GetInputElementDescs(std::vector<D3D11_INPUT_ELEMENT_DESC>& outInputElementDescs) noexcept
	{
		outInputElementDescs.clear();
		outInputElementDescs.reserve(S_TOTAL_INPUT_LAYOUT_DESC_ELEMENTS);

		D3D11_INPUT_ELEMENT_DESC descs[] = {
			{ "VertexPos3D", 0u, DXGI_FORMAT_R32G32B32_FLOAT, 0u, 0u, D3D11_INPUT_PER_VERTEX_DATA, 0u }
		};

		outInputElementDescs.assign(std::begin(descs), std::end(descs));
	}
}