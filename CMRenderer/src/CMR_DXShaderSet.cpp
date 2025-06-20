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
		case DXShaderSetType::QUAD:
			DXShaderSetQuad::GetInputElementDescs(outInputElementDescs);
			return;
		case DXShaderSetType::QUAD_DEPTH:
			DXShaderSetQuadDepth::GetInputElementDescs(outInputElementDescs);
			return;
		case DXShaderSetType::CIRCLE:
			DXShaderSetCircle::GetInputElementDescs(outInputElementDescs);
			return;
		default:
			return;
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

		hResult = deviceRef->CreateVertexShader(
			VertexData.pBytecode->GetBufferPointer(),
			VertexData.pBytecode->GetBufferSize(),
			nullptr,
			&pVertexShader
		);

		cmLoggerRef.LogFatalNLIf(
			hResult != S_OK,
			L"IDXShaderSet [CreateMandatoryShaders] | Failed to create vertex shader."
		);

		hResult = deviceRef->CreatePixelShader(
			PixelData.pBytecode->GetBufferPointer(),
			PixelData.pBytecode->GetBufferSize(),
			nullptr,
			&pPixelShader
		);

		cmLoggerRef.LogFatalNLIf(
			hResult != S_OK, 
			L"IDXShaderSet [CreateMandatoryShaders] | Failed to create pixel shader."
		);

		CreatedMandatoryShaders = true;
	}

	DXShaderSetQuad::DXShaderSetQuad(const DXShaderData& vertexDataRef, const DXShaderData& pixelDataRef) noexcept
		: IDXShaderSet(DXShaderSetType::QUAD, vertexDataRef, pixelDataRef)
	{
	}

	void DXShaderSetQuad::CreateShaderSet(Components::DXDevice& deviceRef, CMCommon::CMLoggerWide& cmLoggerRef) noexcept
	{
		CreateMandatoryShaders(deviceRef, cmLoggerRef);

		CreateInputLayout<DXShaderSetQuad>(deviceRef, cmLoggerRef);

		cmLoggerRef.LogInfoNL(L"DXShaderSetCMRect [CreateShaderSet] | Created shader set.");

		IsCreated = true;
	}

	void DXShaderSetQuad::GetInputElementDescs(std::vector<D3D11_INPUT_ELEMENT_DESC>& outInputElementDescs) noexcept
	{
		outInputElementDescs.clear();
		outInputElementDescs.reserve(S_TOTAL_INPUT_LAYOUT_DESC_ELEMENTS);

		D3D11_INPUT_ELEMENT_DESC descs[] = {
			{ "Pos2D", 0u, DXGI_FORMAT_R32G32_FLOAT, 0u, 0u, D3D11_INPUT_PER_VERTEX_DATA, 0u }
		};

		outInputElementDescs.assign(std::begin(descs), std::end(descs));
	}

	DXShaderSetQuadDepth::DXShaderSetQuadDepth(const DXShaderData& vertexDataRef, const DXShaderData& pixelDataRef) noexcept
		: IDXShaderSet(DXShaderSetType::QUAD_DEPTH, vertexDataRef, pixelDataRef)
	{
	}

	void DXShaderSetQuadDepth::CreateShaderSet(Components::DXDevice& deviceRef, CMCommon::CMLoggerWide& cmLoggerRef) noexcept
	{
		CreateMandatoryShaders(deviceRef, cmLoggerRef);

		CreateInputLayout<DXShaderSetQuadDepth>(deviceRef, cmLoggerRef);

		cmLoggerRef.LogInfoNL(L"DXShaderSetCMCube [CreateShaderSet] | Created shader set.");

		IsCreated = true;
	}

	void DXShaderSetQuadDepth::GetInputElementDescs(std::vector<D3D11_INPUT_ELEMENT_DESC>& outInputElementDescs) noexcept
	{
		outInputElementDescs.clear();
		outInputElementDescs.reserve(S_TOTAL_INPUT_LAYOUT_DESC_ELEMENTS);

		D3D11_INPUT_ELEMENT_DESC descs[] = {
			{ "Pos3D", 0u, DXGI_FORMAT_R32G32B32_FLOAT, 0u, 0u, D3D11_INPUT_PER_VERTEX_DATA, 0u }
		};

		outInputElementDescs.assign(std::begin(descs), std::end(descs));
	}

	DXShaderSetCircle::DXShaderSetCircle(const DXShaderData& vertexDataRef, const DXShaderData& pixelDataRef) noexcept
		: IDXShaderSet(DXShaderSetType::CIRCLE, vertexDataRef, pixelDataRef)
	{
	}

	void DXShaderSetCircle::CreateShaderSet(Components::DXDevice& deviceRef, CMCommon::CMLoggerWide& cmLoggerRef) noexcept
	{
		CreateMandatoryShaders(deviceRef, cmLoggerRef);

		CreateInputLayout<DXShaderSetCircle>(deviceRef, cmLoggerRef);

		cmLoggerRef.LogInfoNL(L"DXShaderSetCMCircle [CreateShaderSet] | Created shader set.");

		IsCreated = true;
	}

	void DXShaderSetCircle::GetInputElementDescs(std::vector<D3D11_INPUT_ELEMENT_DESC>& outInputElementDescs) noexcept
	{
		outInputElementDescs.clear();
		outInputElementDescs.reserve(S_TOTAL_INPUT_LAYOUT_DESC_ELEMENTS);

		D3D11_INPUT_ELEMENT_DESC descs[] = {
			{ "Pos3D", 0u, DXGI_FORMAT_R32G32B32_FLOAT, 0u, 0u, D3D11_INPUT_PER_VERTEX_DATA, 0u },
			{ "InstanceRadius", 0u, DXGI_FORMAT_R32_FLOAT, 1u, 0u, D3D11_INPUT_PER_INSTANCE_DATA, 1u },
		};

		outInputElementDescs.assign(std::begin(descs), std::end(descs));
	}
}