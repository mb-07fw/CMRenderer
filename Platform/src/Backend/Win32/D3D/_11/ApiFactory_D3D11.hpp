#pragma once

#include "Backend/Win32/D3D/_11/ApiResources_D3D11.hpp"
#include "IApiFactory.hpp"

namespace Platform::Backend::Win32::D3D::_11
{
	class Api;

	class ApiFactory final : public IApiFactory
	{
	public:
		ApiFactory(Api& api) noexcept;
		~ApiFactory() noexcept;

		virtual [[nodiscard]] InputLayout* CreateInputLayout() noexcept override;
		virtual [[nodiscard]] VertexShader* CreateVertexShader() noexcept override;
		virtual [[nodiscard]] PixelShader* CreatePixelShader() noexcept override;

        virtual [[nodiscard]] IBuffer* CreateBuffer(
            BufferType type,
			BufferPolicy policy
        ) noexcept override;

        virtual [[nodiscard]] VertexBuffer* CreateVertexBuffer(
            BufferPolicy policy = BufferPolicy::Default
        ) noexcept override;

		virtual [[nodiscard]] IndexBuffer* CreateIndexBuffer(
			BufferPolicy policy = BufferPolicy::Default
		) noexcept override;

        virtual [[nodiscard]] ConstantBuffer* CreateConstantBuffer(
            BufferPolicy policy = BufferPolicy::Default
        ) noexcept override;

		virtual [[nodiscard]] Blob* CreateBlob() noexcept override;
	private:
		Api& m_ApiRef;
	};
}