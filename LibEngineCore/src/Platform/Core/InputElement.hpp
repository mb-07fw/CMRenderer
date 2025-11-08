#pragma once

#include <cstdint>
#include <string_view>

namespace CMEngine
{
	enum class DataFormat : int8_t
	{
		Unspecified = -1,

		UInt8,
		UInt8x2,
		UInt8x3,
		UInt8x4,

		Int8,
		Int8x2,
		Int8x3,
		Int8x4,

		UInt16,
		UInt16x2,
		UInt16x3,
		UInt16x4,

		Int16,
		Int16x2,
		Int16x3,
		Int16x4,

		UInt32,
		UInt32x2,
		UInt32x3,
		UInt32x4,

		Int32,
		Int32x2,
		Int32x3,
		Int32x4,

		Float32,
		Float32x2,
		Float32x3,
		Float32x4,

		/* Set to a 4x4 of 32-bit floating point values. */
		Mat4,

		Count
	};

	enum class InputClass : int8_t
	{
		Invalid = -1,
		PerVertex,
		PerInstance
	};

	inline constexpr [[nodiscard]] std::string_view DataFormatToString(DataFormat format) noexcept;
	inline constexpr [[nodiscard]] std::string_view InputClassToString(InputClass inputClass) noexcept;
	inline constexpr [[nodiscard]] size_t BytesOfFormat(DataFormat format) noexcept;
	inline constexpr [[nodiscard]] bool IsMatrixFormat(DataFormat format) noexcept;
	inline constexpr [[nodiscard]] size_t RowsOfMatrixFormat(DataFormat format) noexcept;
	inline constexpr [[nodiscard]] size_t BytesOfMatrixRow(DataFormat format) noexcept;

	inline constexpr uint32_t G_InputElement_InferByteOffset = ~static_cast<uint32_t>(0);

	/* Used for matrix DataFormat's (Mat4) to clearly indicate that a single input element
		should be expanded for each row in it's matrix type.

		ie., An InputElement with DataFormat::Mat4 with index
			 `G_InputElement_ExpandAsMultiple` will be expanded to
			 elements with the same name, but sequential indices to match
			 hlsl.

		ex)  InputElement("TRANSFORM", G_InputElement_ExpandAsMultiple, DataFormat::Mat4, ..., ..., ..., ...);

			 to -

			  API_Specific_InputElement("TRANSFORM", 0, Float32x4, ..., ..., ..., ...)
			  API_Specific_InputElement("TRANSFORM", 1, Float32x4, ..., ..., ..., ...)
			  API_Specific_InputElement("TRANSFORM", 2, Float32x4, ..., ..., ..., ...)
			  API_Specific_InputElement("TRANSFORM", 3, Float32x4, ..., ..., ..., ...)
	 */
	inline constexpr uint32_t G_InputElement_ExpandAsMultiple = ~static_cast<uint32_t>(0) - 1;

	struct InputElement
	{

		inline constexpr InputElement(
			std::string_view name,
			uint32_t index,
			DataFormat format,
			uint32_t inputSlot,
			uint32_t alignedByteOffset = G_InputElement_InferByteOffset,
			InputClass inputClass = InputClass::PerVertex,
			uint32_t instanceStepRate = 0
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

		std::string_view Name;
		uint32_t Index = 0;
		DataFormat Format = DataFormat::Unspecified;
		uint32_t InputSlot = 0;

		/* Offset in bytes from previous InputElement. */
		uint32_t AlignedByteOffset = G_InputElement_InferByteOffset;
		InputClass InputClass = InputClass::Invalid;

		/* The number of instances to draw using the same per-instance
		 * data before advancing in the buffer by one element. */
		uint32_t InstanceStepRate = 0;
	};

	inline constexpr [[nodiscard]] std::string_view DataFormatToString(DataFormat format) noexcept
	{
		switch (format)
		{
		case DataFormat::Unspecified: return std::string_view("Unspecified");
		case DataFormat::Count:       return std::string_view("Count");
		default:

		case DataFormat::Int8:		  return std::string_view("Int8");
		case DataFormat::UInt8:		  return std::string_view("UInt8");

		case DataFormat::Int8x2:	  return std::string_view("Int8x2");
		case DataFormat::UInt8x2:     return std::string_view("UInt8x2");

		case DataFormat::Int8x3:	  return std::string_view("Int8x3");
		case DataFormat::UInt8x3:	  return std::string_view("UInt8x3");

		case DataFormat::Int8x4:      return std::string_view("Int8x4");
		case DataFormat::UInt8x4:	  return std::string_view("UInt8x4");

		case DataFormat::Int16:       return std::string_view("Int16");
		case DataFormat::UInt16:	  return std::string_view("UInt16");

		case DataFormat::Int16x2:     return std::string_view("Int16x2");
		case DataFormat::UInt16x2:	  return std::string_view("UInt16x2");

		case DataFormat::Int16x3:     return std::string_view("Int16x3");
		case DataFormat::UInt16x3:	  return std::string_view("UInt16x3");

		case DataFormat::Int16x4:     return std::string_view("Int16x4");
		case DataFormat::UInt16x4:	  return std::string_view("UInt16x4");

		case DataFormat::Int32:       return std::string_view("Int32");
		case DataFormat::UInt32:      return std::string_view("UInt32");

		case DataFormat::Int32x2:     return std::string_view("Int32x2");
		case DataFormat::UInt32x2:    return std::string_view("UInt32x2");

		case DataFormat::Int32x3:     return std::string_view("Int32x3");
		case DataFormat::UInt32x3:    return std::string_view("UInt32x3");

		case DataFormat::Int32x4:     return std::string_view("Int32x4");
		case DataFormat::UInt32x4:    return std::string_view("UInt32x4");

		case DataFormat::Float32:	  return std::string_view("Float32");
		case DataFormat::Float32x2:   return std::string_view("Float32x2");
		case DataFormat::Float32x3:   return std::string_view("Float32x3");
		case DataFormat::Float32x4:   return std::string_view("Float32x4");

		case DataFormat::Mat4:		  return std::string_view("Mat4");
		}
	}

	inline constexpr [[nodiscard]] std::string_view InputClassToString(InputClass inputClass) noexcept
	{
		switch (inputClass)
		{
		default: [[fallthrough]];
		case InputClass::Invalid:     return std::string_view("Invalid");
		case InputClass::PerVertex:   return std::string_view("PerVertex");
		case InputClass::PerInstance: return std::string_view("PerInstance");
		}
	}

	inline constexpr [[nodiscard]] size_t BytesOfFormat(DataFormat format) noexcept
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

		/* NOTE: It is up to the caller to know that this is a matrix format, as there is no
			corresponding DXGI_FORMAT. The data format for each row is returned. */
		case DataFormat::Mat4:		  return BytesOfFormat(DataFormat::Float32x4) * 4;
		}
	}

	inline constexpr [[nodiscard]] bool IsMatrixFormat(DataFormat format) noexcept
	{
		switch (format)
		{
		default:			   return false;
		case DataFormat::Mat4: return true;
		}
	}

	inline constexpr [[nodiscard]] size_t RowsOfMatrixFormat(DataFormat format) noexcept
	{
		switch (format)
		{
		default:			   return 0;
		case DataFormat::Mat4: return 4;
		}
	}

	inline constexpr [[nodiscard]] size_t BytesOfMatrixRow(DataFormat format) noexcept
	{
		switch (format)
		{
		default:			   return 0;
		case DataFormat::Mat4: return BytesOfFormat(DataFormat::Float32x4);
		}
	}
}