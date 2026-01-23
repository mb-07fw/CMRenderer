#include "IApiResources.hpp"
#include "Common/Cast.hpp"

namespace Platform
{
	PLATFORM_API [[nodiscard]] StringView DataFormatToString(DataFormat format) noexcept
	{
		switch (format)
		{
		case DataFormat::Unspecified:  return StringView("Unspecified");
		case DataFormat::Count:        return StringView("Count");

		case DataFormat::Int8:		   return StringView("Int8");
		case DataFormat::UInt8:		   return StringView("UInt8");

		case DataFormat::Int8x2:	   return StringView("Int8x2");
		case DataFormat::UInt8x2:      return StringView("UInt8x2");

		case DataFormat::Int8x3:	   return StringView("Int8x3");
		case DataFormat::UInt8x3:	   return StringView("UInt8x3");

		case DataFormat::Int8x4:       return StringView("Int8x4");
		case DataFormat::UInt8x4:	   return StringView("UInt8x4");

		case DataFormat::Int16:        return StringView("Int16");
		case DataFormat::UInt16:	   return StringView("UInt16");

		case DataFormat::Int16x2:      return StringView("Int16x2");
		case DataFormat::UInt16x2:	   return StringView("UInt16x2");

		case DataFormat::Int16x3:      return StringView("Int16x3");
		case DataFormat::UInt16x3:	   return StringView("UInt16x3");

		case DataFormat::Int16x4:      return StringView("Int16x4");
		case DataFormat::UInt16x4:	   return StringView("UInt16x4");

		case DataFormat::Int32:        return StringView("Int32");
		case DataFormat::UInt32:       return StringView("UInt32");

		case DataFormat::Int32x2:      return StringView("Int32x2");
		case DataFormat::UInt32x2:     return StringView("UInt32x2");
 
		case DataFormat::Int32x3:      return StringView("Int32x3");
		case DataFormat::UInt32x3:     return StringView("UInt32x3");

		case DataFormat::Int32x4:      return StringView("Int32x4");
		case DataFormat::UInt32x4:     return StringView("UInt32x4");

		case DataFormat::Float32:	   return StringView("Float32");
		case DataFormat::Float32x2:    return StringView("Float32x2");
		case DataFormat::Float32x3:    return StringView("Float32x3");
		case DataFormat::Float32x4:    return StringView("Float32x4");

		case DataFormat::Mat4_Float32: return StringView("Mat4_Float32");
		default:					   return StringView("!!!INVALID!!!");
		}
	}

	PLATFORM_API [[nodiscard]] StringView InputClassToString(InputClass inputClass) noexcept
	{
		switch (inputClass)
		{
		default: [[fallthrough]];
		case InputClass::Invalid:     return StringView("Invalid");
		case InputClass::PerVertex:   return StringView("PerVertex");
		case InputClass::PerInstance: return StringView("PerInstance");
		}
	}

	PLATFORM_API [[nodiscard]] size_t BytesOfDataFormat(DataFormat format) noexcept
	{
		switch (format)
		{
		case DataFormat::Unspecified: [[fallthrough]];
		case DataFormat::Count:       [[fallthrough]];
		default:					  return 0;

		case DataFormat::Int8:        [[fallthrough]];
		case DataFormat::UInt8:		  return sizeof(uint8_t);

		case DataFormat::Int8x2:      [[fallthrough]];
		case DataFormat::UInt8x2:	  return sizeof(uint8_t) * 2;

		case DataFormat::Int8x3:      [[fallthrough]];
		case DataFormat::UInt8x3:	  return sizeof(uint8_t) * 3;

		case DataFormat::Int8x4:      [[fallthrough]];
		case DataFormat::UInt8x4:	  return sizeof(uint8_t) * 4;

		case DataFormat::Int16:       [[fallthrough]];
		case DataFormat::UInt16:	  return sizeof(uint16_t);

		case DataFormat::Int16x2:     [[fallthrough]];
		case DataFormat::UInt16x2:	  return sizeof(uint16_t) * 2;

		case DataFormat::Int16x3:     [[fallthrough]];
		case DataFormat::UInt16x3:	  return sizeof(uint16_t) * 3;

		case DataFormat::Int16x4:     [[fallthrough]];
		case DataFormat::UInt16x4:	  return sizeof(uint16_t) * 4;

		case DataFormat::Int32:       [[fallthrough]];
		case DataFormat::UInt32:      return sizeof(uint32_t);

		case DataFormat::Int32x2:     [[fallthrough]];
		case DataFormat::UInt32x2:    return sizeof(uint32_t) * 2;

		case DataFormat::Int32x3:     [[fallthrough]];
		case DataFormat::UInt32x3:    return sizeof(uint32_t) * 3;

		case DataFormat::Int32x4:     [[fallthrough]];
		case DataFormat::UInt32x4:    return sizeof(uint32_t) * 4;

		case DataFormat::Float32:	  return sizeof(float);
		case DataFormat::Float32x2:   return sizeof(float) * 2;
		case DataFormat::Float32x3:   return sizeof(float) * 3;
		case DataFormat::Float32x4:   return sizeof(float) * 4;

		/* NOTE: The data format for each row is returned. */
		case DataFormat::Mat4_Float32: return BytesOfDataFormat(DataFormat::Float32x4) * 4;
		}
	}

	PLATFORM_API [[nodiscard]] bool IsMatrixDataFormat(DataFormat format) noexcept
	{
		switch (format)
		{
		default:			           return false;
		case DataFormat::Mat4_Float32: return true;
		}
	}

	PLATFORM_API [[nodiscard]] size_t RowsOfMatrixDataFormat(DataFormat format) noexcept
	{
		switch (format)
		{
		default:			           return 0;
		case DataFormat::Mat4_Float32: return 4;
		}
	}

	PLATFORM_API [[nodiscard]] DataFormat DataFormatOfMatrixRow(DataFormat matrixFormat) noexcept
	{
		switch (matrixFormat)
		{
		default:					   return DataFormat::Unspecified;
		case DataFormat::Mat4_Float32: return DataFormat::Float32x4;
		}
	}

	InputElement::InputElement(
		const StringView& name,
		uint32_t index,
		DataFormat format,
		uint32_t inputSlot,
		uint32_t alignedByteOffset,
		Platform::InputClass inputClass,
		uint32_t instanceStepRate
	) noexcept
		: Name(name),
		  Index(index),
		  Format(format),
		  InputSlot(inputSlot),
		  AlignedByteOffset(alignedByteOffset),
		  InputClass(inputClass),
		  InstanceStepRate(instanceStepRate)
	{
	}
}