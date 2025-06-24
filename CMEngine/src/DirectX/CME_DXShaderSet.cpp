#include "Core/CME_PCH.hpp"
#include "DirectX/CME_DXShaderSet.hpp"

namespace CMEngine::DirectXAPI::DX11
{
	IDXShaderSet::IDXShaderSet(DXShaderSetType type, const DXShaderData& vertexData, const DXShaderData& pixelData) noexcept
		: Type(type), VertexData(vertexData), PixelData(pixelData)
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

	void IDXShaderSet::CreateMandatoryShaders(DXDevice& device, CMCommon::CMLoggerWide& logger) noexcept
	{
		logger.LogFatalNLIf(
			!device.IsCreated(),
			L"IDXShaderSet [CreateMandatoryShaders] | Device isn't created."
		);

		logger.LogFatalNLIf(
			VertexData.pBytecode.Get() == nullptr,
			L"IDXShaderSet [CreateMandatoryShaders] | Vertex shader bytecode isn't present."
		);

		logger.LogFatalNLIf(
			PixelData.pBytecode.Get() == nullptr,
			L"IDXShaderSet [CreateMandatoryShaders] | Pixel shader bytecode isn't present."
		);

		logger.LogFatalNLIf(
			VertexData.CorrespondingShaderSet == DXShaderSetType::INVALID,
			L"IDXShaderSet[CreateMandatoryShaders] | Vertex data doesn't have a corresponding shader set."
		);

		logger.LogFatalNLIf(
			PixelData.CorrespondingShaderSet == DXShaderSetType::INVALID,
			L"IDXShaderSet[CreateMandatoryShaders] | Pixel data doesn't have a corresponding shader set."
		);

		HRESULT hResult = S_OK;

		hResult = device->CreateVertexShader(
			VertexData.pBytecode->GetBufferPointer(),
			VertexData.pBytecode->GetBufferSize(),
			nullptr,
			&pVertexShader
		);

		logger.LogFatalNLIf(
			hResult != S_OK,
			L"IDXShaderSet [CreateMandatoryShaders] | Failed to create vertex shader."
		);

		hResult = device->CreatePixelShader(
			PixelData.pBytecode->GetBufferPointer(),
			PixelData.pBytecode->GetBufferSize(),
			nullptr,
			&pPixelShader
		);

		logger.LogFatalNLIf(
			hResult != S_OK, 
			L"IDXShaderSet [CreateMandatoryShaders] | Failed to create pixel shader."
		);

		CreatedMandatoryShaders = true;
	}

	DXShaderSetQuad::DXShaderSetQuad(const DXShaderData& vertexData, const DXShaderData& pixelData) noexcept
		: IDXShaderSet(DXShaderSetType::QUAD, vertexData, pixelData)
	{
	}

	void DXShaderSetQuad::CreateShaderSet(DXDevice& device, CMCommon::CMLoggerWide& logger) noexcept
	{
		CreateMandatoryShaders(device, logger);

		CreateInputLayout<DXShaderSetQuad>(device, logger);

		logger.LogInfoNL(L"DXShaderSetCMRect [CreateShaderSet] | Created shader set.");

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

	DXShaderSetQuadDepth::DXShaderSetQuadDepth(const DXShaderData& vertexData, const DXShaderData& pixelData) noexcept
		: IDXShaderSet(DXShaderSetType::QUAD_DEPTH, vertexData, pixelData)
	{
	}

	void DXShaderSetQuadDepth::CreateShaderSet(DXDevice& device, CMCommon::CMLoggerWide& logger) noexcept
	{
		CreateMandatoryShaders(device, logger);

		CreateInputLayout<DXShaderSetQuadDepth>(device, logger);

		logger.LogInfoNL(L"DXShaderSetCMCube [CreateShaderSet] | Created shader set.");

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

	DXShaderSetCircle::DXShaderSetCircle(const DXShaderData& vertexData, const DXShaderData& pixelData) noexcept
		: IDXShaderSet(DXShaderSetType::CIRCLE, vertexData, pixelData)
	{
	}

	void DXShaderSetCircle::CreateShaderSet(DXDevice& device, CMCommon::CMLoggerWide& logger) noexcept
	{
		CreateMandatoryShaders(device, logger);

		CreateInputLayout<DXShaderSetCircle>(device, logger);

		logger.LogInfoNL(L"DXShaderSetCMCircle [CreateShaderSet] | Created shader set.");

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