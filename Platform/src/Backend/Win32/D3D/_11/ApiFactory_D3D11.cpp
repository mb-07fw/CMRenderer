#include "Backend/Win32/D3D/_11/ApiFactory_D3D11.hpp"
#include "Backend/Win32/D3D/_11/Api_D3D11.hpp"
#include "PlatformFailure.hpp"

namespace Platform::Backend::Win32::D3D::_11
{
	ApiFactory::ApiFactory(Api& api) noexcept
		: m_ApiRef(api)
	{
	}

	ApiFactory::~ApiFactory() noexcept
	{
	}

	[[nodiscard]] InputLayout* ApiFactory::CreateInputLayout() noexcept
	{
		return new InputLayout;
	}

	[[nodiscard]] VertexShader* ApiFactory::CreateVertexShader() noexcept
	{
		return new VertexShader;
	}

	[[nodiscard]] PixelShader* ApiFactory::CreatePixelShader() noexcept
	{
		return new PixelShader;
	}

	[[nodiscard]] IBuffer* ApiFactory::CreateBuffer(
		BufferType type,
		BufferPolicy policy
	) noexcept
	{
		switch (type)
		{
		case BufferType::Vertex:   return new VertexBuffer(policy);
		case BufferType::Index:    return new IndexBuffer(policy);
		case BufferType::Constant: return new ConstantBuffer(policy);
		case BufferType::Invalid: [[fallthrough]];
		default:
			PLATFORM_FAILURE_IF(false, "(ApiFactory::CreateBuffer) Unsupported buffer type.");
			return nullptr;
		}
	}

	[[nodiscard]] VertexBuffer* ApiFactory::CreateVertexBuffer(BufferPolicy policy) noexcept
	{
		return new VertexBuffer(policy);
	}

	[[nodiscard]] IndexBuffer* ApiFactory::CreateIndexBuffer(BufferPolicy policy) noexcept
	{
		return new IndexBuffer(policy);
	}

	[[nodiscard]] ConstantBuffer* ApiFactory::CreateConstantBuffer(BufferPolicy policy) noexcept
	{
		return new ConstantBuffer(policy);
	}

	[[nodiscard]] Blob* ApiFactory::CreateBlob() noexcept
	{
		return new Blob;
	}
}