#pragma once

#include "Platform/Core/IUploadable.hpp"
#include "Platform/WinImpl/Types_WinImpl.hpp"

namespace CMEngine::Platform::WinImpl
{
	/* A pure virtual interface for an object that is bindable to the D3D11 pipeline. */
	class IDXUploadable
	{
	public:
		IDXUploadable() = default;
		virtual ~IDXUploadable() = default;

		virtual void Upload(const ComPtr<ID3D11DeviceContext>& pContext) const noexcept = 0;
		virtual void ClearUpload(const ComPtr<ID3D11DeviceContext>& pContext) const noexcept = 0;
	};
}