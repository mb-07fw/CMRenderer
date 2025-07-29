#include "Core/PCH.hpp"
#include "DX/DX11/DX11_ShaderSet.hpp"

namespace CMEngine::DX::DX11
{
	IShaderSet::IShaderSet(ShaderSetType type, const ShaderData& vertexData, const ShaderData& pixelData) noexcept
		: Type(type), VertexData(vertexData), PixelData(pixelData)
	{
	}

	void IShaderSet::GetInputElementDescsOfType(std::vector<D3D11_INPUT_ELEMENT_DESC>& outInputElementDescs, ShaderSetType setType) noexcept
	{
		switch (setType)
		{
		case ShaderSetType::QUAD:
			ShaderSetQuad::GetInputElementDescs(outInputElementDescs);
			return;
		case ShaderSetType::QUAD_OUTLINED:
			ShaderSetQuadOutlined::GetInputElementDescs(outInputElementDescs);
			return;
		case ShaderSetType::QUAD_DEPTH:
			ShaderSetQuadDepth::GetInputElementDescs(outInputElementDescs);
			return;
		case ShaderSetType::CIRCLE:
			ShaderSetCircle::GetInputElementDescs(outInputElementDescs);
			return;
		case ShaderSetType::INVALID: [[fallthrough]];
		default:
			return;
		}
	}

	void IShaderSet::CreateMandatoryShaders(Device& device, Common::LoggerWide& logger) noexcept
	{
		logger.LogFatalNLIf(
			!device.IsCreated(),
			L"IShaderSet [CreateMandatoryShaders] | Device isn't created."
		);

		logger.LogFatalNLIf(
			VertexData.pBytecode.Get() == nullptr,
			L"IShaderSet [CreateMandatoryShaders] | Vertex shader bytecode isn't present."
		);

		logger.LogFatalNLIf(
			PixelData.pBytecode.Get() == nullptr,
			L"IShaderSet [CreateMandatoryShaders] | Pixel shader bytecode isn't present."
		);

		logger.LogFatalNLIf(
			VertexData.CorrespondingShaderSet == ShaderSetType::INVALID,
			L"IShaderSet[CreateMandatoryShaders] | Vertex data doesn't have a corresponding shader set."
		);

		logger.LogFatalNLIf(
			PixelData.CorrespondingShaderSet == ShaderSetType::INVALID,
			L"IShaderSet[CreateMandatoryShaders] | Pixel data doesn't have a corresponding shader set."
		);

		HRESULT hResult = S_OK;

		hResult = device->CreateVertexShader(
			VertexData.pBytecode->GetBufferPointer(),
			VertexData.pBytecode->GetBufferSize(),
			nullptr,
			&pVertexShader
		);

		logger.LogFatalNLIf(
			FAILED(hResult),
			L"IShaderSet [CreateMandatoryShaders] | Failed to create vertex shader."
		);

		hResult = device->CreatePixelShader(
			PixelData.pBytecode->GetBufferPointer(),
			PixelData.pBytecode->GetBufferSize(),
			nullptr,
			&pPixelShader
		);

		logger.LogFatalNLIf(
			FAILED(hResult), 
			L"IShaderSet [CreateMandatoryShaders] | Failed to create pixel shader."
		);

		CreatedMandatoryShaders = true;
	}

	ShaderSetQuad::ShaderSetQuad(const ShaderData& vertexData, const ShaderData& pixelData) noexcept
		: IShaderSet(ShaderSetType::QUAD, vertexData, pixelData)
	{
	}

	void ShaderSetQuad::CreateShaderSet(Device& device, Common::LoggerWide& logger) noexcept
	{
		CreateMandatoryShaders(device, logger);

		CreateInputLayout<ShaderSetQuad>(device, logger);

		logger.LogInfoNL(L"ShaderSetRect [CreateShaderSet] | Created shader set.");

		IsCreated = true;
	}

	void ShaderSetQuad::GetInputElementDescs(std::vector<D3D11_INPUT_ELEMENT_DESC>& outInputElementDescs) noexcept
	{
		outInputElementDescs.clear();
		outInputElementDescs.reserve(S_TOTAL_INPUT_LAYOUT_DESC_ELEMENTS);

		D3D11_INPUT_ELEMENT_DESC descs[] = {
			{ "Pos2D", 0u, DXGI_FORMAT_R32G32_FLOAT, 0u, 0u, D3D11_INPUT_PER_VERTEX_DATA, 0u }
		};

		outInputElementDescs.assign(std::begin(descs), std::end(descs));
	}

	ShaderSetQuadOutlined::ShaderSetQuadOutlined(const ShaderData& vertexData, const ShaderData& pixelData) noexcept
		: IShaderSet(ShaderSetType::QUAD_OUTLINED, vertexData, pixelData)
	{
	}

	void ShaderSetQuadOutlined::CreateShaderSet(Device& device, Common::LoggerWide& logger) noexcept
	{
		CreateMandatoryShaders(device, logger);

		CreateInputLayout<ShaderSetQuadOutlined>(device, logger);

		logger.LogInfoNL(L"ShaderSetQuadOutlined [CreateShaderSet] | Created shader set.");

		IsCreated = true;
	}

	void ShaderSetQuadOutlined::GetInputElementDescs(std::vector<D3D11_INPUT_ELEMENT_DESC>& outInputElementDescs) noexcept
	{
		outInputElementDescs.clear();
		outInputElementDescs.reserve(S_TOTAL_INPUT_LAYOUT_DESC_ELEMENTS);

		D3D11_INPUT_ELEMENT_DESC descs[] = {
			{ "Screen_Pos2D", 0u, DXGI_FORMAT_R32G32_FLOAT, 0u, 0u, D3D11_INPUT_PER_VERTEX_DATA, 0u }
		};

		outInputElementDescs.assign(std::begin(descs), std::end(descs));
	}

	ShaderSetQuadDepth::ShaderSetQuadDepth(const ShaderData& vertexData, const ShaderData& pixelData) noexcept
		: IShaderSet(ShaderSetType::QUAD_DEPTH, vertexData, pixelData)
	{
	}

	void ShaderSetQuadDepth::CreateShaderSet(Device& device, Common::LoggerWide& logger) noexcept
	{
		CreateMandatoryShaders(device, logger);

		CreateInputLayout<ShaderSetQuadDepth>(device, logger);

		logger.LogInfoNL(L"ShaderSetCube [CreateShaderSet] | Created shader set.");

		IsCreated = true;
	}

	void ShaderSetQuadDepth::GetInputElementDescs(std::vector<D3D11_INPUT_ELEMENT_DESC>& outInputElementDescs) noexcept
	{
		outInputElementDescs.clear();
		outInputElementDescs.reserve(S_TOTAL_INPUT_LAYOUT_DESC_ELEMENTS);

		D3D11_INPUT_ELEMENT_DESC descs[] = {
			{ "Pos3D", 0u, DXGI_FORMAT_R32G32B32_FLOAT, 0u, 0u, D3D11_INPUT_PER_VERTEX_DATA, 0u }
		};

		outInputElementDescs.assign(std::begin(descs), std::end(descs));
	}

	ShaderSetCircle::ShaderSetCircle(const ShaderData& vertexData, const ShaderData& pixelData) noexcept
		: IShaderSet(ShaderSetType::CIRCLE, vertexData, pixelData)
	{
	}

	void ShaderSetCircle::CreateShaderSet(Device& device, Common::LoggerWide& logger) noexcept
	{
		CreateMandatoryShaders(device, logger);

		CreateInputLayout<ShaderSetCircle>(device, logger);

		logger.LogInfoNL(L"ShaderSetCircle [CreateShaderSet] | Created shader set.");

		IsCreated = true;
	}

	void ShaderSetCircle::GetInputElementDescs(std::vector<D3D11_INPUT_ELEMENT_DESC>& outInputElementDescs) noexcept
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