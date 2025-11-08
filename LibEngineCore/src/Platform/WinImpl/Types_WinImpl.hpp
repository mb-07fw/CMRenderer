#pragma once

#include "Platform/Core/InputElement.hpp"
#include "Types.hpp"

#include <spdlog/spdlog.h>

#include <d3d11.h>
#include <wrl/client.h>

namespace CMEngine::Platform::WinImpl
{
	template <typename Ty>
	using ComPtr = Microsoft::WRL::ComPtr<Ty>;

	inline constexpr [[nodiscard]] size_t BytesOfFormat(DXGI_FORMAT format) noexcept;
	inline constexpr [[nodiscard]] DXGI_FORMAT DataToDXGI(DataFormat format) noexcept;
	inline constexpr [[nodiscard]] D3D11_INPUT_CLASSIFICATION InputClassToD3D11(InputClass inputClass) noexcept;

	inline constexpr [[nodiscard]] size_t BytesOfFormat(DXGI_FORMAT format) noexcept
	{
		switch (format)
		{
		case DXGI_FORMAT_R8_UINT:				[[fallthrough]];
		case DXGI_FORMAT_R8_SINT:				[[fallthrough]];
		case DXGI_FORMAT_R8_TYPELESS:			[[fallthrough]];
		case DXGI_FORMAT_R8_SNORM:              [[fallthrough]];
		case DXGI_FORMAT_R8_UNORM:				return sizeof(std::byte);

		case DXGI_FORMAT_R8G8_UINT:				[[fallthrough]];
		case DXGI_FORMAT_R8G8_SINT:				[[fallthrough]];
		case DXGI_FORMAT_R8G8_TYPELESS:			[[fallthrough]];
		case DXGI_FORMAT_R8G8_SNORM:			[[fallthrough]];
		case DXGI_FORMAT_R8G8_UNORM:			return sizeof(std::byte) * 2;

		case DXGI_FORMAT_R8G8B8A8_UINT:			[[fallthrough]];
		case DXGI_FORMAT_R8G8B8A8_SINT:			[[fallthrough]];
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:		[[fallthrough]];
		case DXGI_FORMAT_R8G8B8A8_SNORM:		[[fallthrough]];
		case DXGI_FORMAT_R8G8B8A8_UNORM:		return sizeof(std::byte) * 4;

		case DXGI_FORMAT_R16_UINT:				[[fallthrough]];
		case DXGI_FORMAT_R16_SINT:				[[fallthrough]];
		case DXGI_FORMAT_R16_FLOAT:				[[fallthrough]];
		case DXGI_FORMAT_R16_TYPELESS:			[[fallthrough]];
		case DXGI_FORMAT_R16_SNORM:             [[fallthrough]];
		case DXGI_FORMAT_R16_UNORM:				return sizeof(uint16_t);

		case DXGI_FORMAT_R16G16_UINT:			[[fallthrough]];
		case DXGI_FORMAT_R16G16_SINT:			[[fallthrough]];
		case DXGI_FORMAT_R16G16_FLOAT:			[[fallthrough]];
		case DXGI_FORMAT_R16G16_TYPELESS:		[[fallthrough]];
		case DXGI_FORMAT_R16G16_SNORM:			[[fallthrough]];
		case DXGI_FORMAT_R16G16_UNORM:			return sizeof(uint16_t) * 2;

		case DXGI_FORMAT_R16G16B16A16_UINT:		[[fallthrough]];
		case DXGI_FORMAT_R16G16B16A16_SINT:		[[fallthrough]];
		case DXGI_FORMAT_R16G16B16A16_FLOAT:	[[fallthrough]];
		case DXGI_FORMAT_R16G16B16A16_TYPELESS: [[fallthrough]];
		case DXGI_FORMAT_R16G16B16A16_SNORM:	[[fallthrough]];
		case DXGI_FORMAT_R16G16B16A16_UNORM:	return sizeof(uint16_t) * 4;

		case DXGI_FORMAT_R32_UINT:				[[fallthrough]];
		case DXGI_FORMAT_R32_SINT:				[[fallthrough]];
		case DXGI_FORMAT_R32_FLOAT:				[[fallthrough]];
		case DXGI_FORMAT_R32_TYPELESS:			return sizeof(uint32_t);

		case DXGI_FORMAT_R32G32_UINT:           [[fallthrough]];
		case DXGI_FORMAT_R32G32_SINT:           [[fallthrough]];
		case DXGI_FORMAT_R32G32_FLOAT:          [[fallthrough]];
		case DXGI_FORMAT_R32G32_TYPELESS:       return sizeof(uint32_t) * 2;

		case DXGI_FORMAT_R32G32B32_UINT:        [[fallthrough]];
		case DXGI_FORMAT_R32G32B32_SINT:        [[fallthrough]];
		case DXGI_FORMAT_R32G32B32_FLOAT:       [[fallthrough]];
		case DXGI_FORMAT_R32G32B32_TYPELESS:    return sizeof(uint32_t) * 3;

		case DXGI_FORMAT_R32G32B32A32_UINT:     [[fallthrough]];
		case DXGI_FORMAT_R32G32B32A32_SINT:     [[fallthrough]];
		case DXGI_FORMAT_R32G32B32A32_FLOAT:    [[fallthrough]];
		case DXGI_FORMAT_R32G32B32A32_TYPELESS: return sizeof(uint32_t) * 4;

		default:
			spdlog::warn("(WinImpl::BytesOfFormat) Unsupported or unimplemented DataFormat was provided: {}", (uint32_t)format);
			return 0;
		}
	}


	inline constexpr [[nodiscard]] DXGI_FORMAT DataToDXGI(DataFormat format) noexcept
	{
		switch (format)
		{
		case DataFormat::Unspecified: [[fallthrough]];
		case DataFormat::Count:		  return DXGI_FORMAT_UNKNOWN;

		case DataFormat::Int8:		  return DXGI_FORMAT_R8_SINT;
		case DataFormat::UInt8:		  return DXGI_FORMAT_R8_UINT;

		case DataFormat::Int8x2:	  return DXGI_FORMAT_R8G8_SINT;
		case DataFormat::UInt8x2:	  return DXGI_FORMAT_R8G8_UINT;

		case DataFormat::Int8x3:      [[fallthrough]];
		case DataFormat::Int8x4:	  return DXGI_FORMAT_R8G8B8A8_SINT;

		case DataFormat::UInt8x3:     [[fallthrough]];
		case DataFormat::UInt8x4:	  return DXGI_FORMAT_R8G8B8A8_UINT;

		case DataFormat::Int16:		  return DXGI_FORMAT_R16_SINT;
		case DataFormat::UInt16:	  return DXGI_FORMAT_R16_UINT;

		case DataFormat::Int16x2:	  return DXGI_FORMAT_R16G16_SINT;
		case DataFormat::UInt16x2:	  return DXGI_FORMAT_R16G16_UINT;

		case DataFormat::Int16x3:	  [[fallthrough]];
		case DataFormat::Int16x4:	  return DXGI_FORMAT_R16G16B16A16_SINT;

		case DataFormat::UInt16x3:	  [[fallthrough]];
		case DataFormat::UInt16x4:	  return DXGI_FORMAT_R16G16B16A16_UINT;

		case DataFormat::UInt32:	  return DXGI_FORMAT_R32_UINT;
		case DataFormat::UInt32x2:	  return DXGI_FORMAT_R32G32_UINT;
		case DataFormat::UInt32x3:    return DXGI_FORMAT_R32G32B32_UINT;
		case DataFormat::UInt32x4:    return DXGI_FORMAT_R32G32B32A32_UINT;

		case DataFormat::Float32:	  return DXGI_FORMAT_R32_FLOAT;
		case DataFormat::Float32x2:   return DXGI_FORMAT_R32G32_FLOAT;
		case DataFormat::Float32x3:   return DXGI_FORMAT_R32G32B32_FLOAT;
		case DataFormat::Float32x4:   return DXGI_FORMAT_R32G32B32A32_FLOAT;
					 
		/* It is up to the caller to know that this is a matrix format, as there is no 
		  corresponding DXGI_FORMAT. The data format for each row is returned. */
		case DataFormat::Mat4:	      return DXGI_FORMAT_R32G32B32A32_FLOAT;

		default:
			spdlog::warn("(WinImpl::DataToDXGI) Unsupported or unimplemented DataFormat was provided: {}", (uint32_t)format);
			return DXGI_FORMAT_UNKNOWN;
		}
	}

	inline constexpr [[nodiscard]] D3D11_INPUT_CLASSIFICATION InputClassToD3D11(InputClass inputClass) noexcept
	{
		switch (inputClass)
		{
		case InputClass::Invalid: [[fallthrough]];
		default: 
			return static_cast<D3D11_INPUT_CLASSIFICATION>(-1);

		case InputClass::PerVertex:   return D3D11_INPUT_PER_VERTEX_DATA;
		case InputClass::PerInstance: return D3D11_INPUT_PER_INSTANCE_DATA;
		}
	}
}