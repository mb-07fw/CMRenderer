#pragma once

#include "Export.hpp"
#include "IRefCounted.hpp"
#include "Types.hpp"

#include <cstdint>

namespace Platform
{
#pragma region InputElement
	enum class PLATFORM_API DataFormat : int8_t
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

		Mat4_Float32,

		Count
	};

	enum class PLATFORM_API InputClass : int8_t
	{
		Invalid = -1,
		PerVertex,
		PerInstance
	};

	PLATFORM_API [[nodiscard]] StringView DataFormatToString(DataFormat format) noexcept;
	PLATFORM_API [[nodiscard]] StringView InputClassToString(InputClass inputClass) noexcept;
	PLATFORM_API [[nodiscard]] size_t BytesOfDataFormat(DataFormat format) noexcept;
	PLATFORM_API [[nodiscard]] bool IsMatrixDataFormat(DataFormat format) noexcept;
	PLATFORM_API [[nodiscard]] size_t RowsOfMatrixDataFormat(DataFormat matrixFormat) noexcept;
	PLATFORM_API [[nodiscard]] DataFormat DataFormatOfMatrixRow(DataFormat matrixFormat) noexcept;

	inline constexpr uint32_t G_InputElement_InferByteOffset = ~static_cast<uint32_t>(0);

	/* Used for matrix DataFormat's (Mat4) to clearly indicate that a single input element
		should be expanded for each row in it's matrix type.

		ie., An InputElement with DataFormat::Mat4_... with index
			 `G_InputElement_ExpandAsMultiple` will be expanded to
			 elements with the same name, but sequential indices to
			 match hlsl.

		ex)  InputElement("TRANSFORM", G_InputElement_ExpandAsMultiple, DataFormat::Mat4_Float32, ..., ..., ..., ...);

			 to -

			 API_Specific_InputElement("TRANSFORM", 0, Float32x4, ..., ..., ..., ...)
			 API_Specific_InputElement("TRANSFORM", 1, Float32x4, ..., ..., ..., ...)
			 API_Specific_InputElement("TRANSFORM", 2, Float32x4, ..., ..., ..., ...)
			 API_Specific_InputElement("TRANSFORM", 3, Float32x4, ..., ..., ..., ...)
	 */
	inline constexpr uint32_t G_InputElement_ExpandAsMultiple = (~static_cast<uint32_t>(0)) - 1;

	struct PLATFORM_API InputElement
	{
		InputElement() = default;
		~InputElement() = default;

		InputElement(
			const StringView& name,
			uint32_t index,
			DataFormat format,
			uint32_t inputSlot,
			uint32_t alignedByteOffset = G_InputElement_InferByteOffset,
			InputClass inputClass = InputClass::PerVertex,
			uint32_t instanceStepRate = 0
		) noexcept;

		InputElement(const InputElement&) = default;
		InputElement(InputElement&& other) = default;
		InputElement& operator=(const InputElement&) = default;
		InputElement& operator=(InputElement&&) = default;
		 
		String Name; // Semantic name
		uint32_t Index = 0; // Semantic index
		DataFormat Format = DataFormat::Unspecified;
		uint32_t InputSlot = 0; // Vertex buffer slot that data will be present in.

		/* Offset in bytes from previous InputElement. */
		uint32_t AlignedByteOffset = G_InputElement_InferByteOffset;
		InputClass InputClass = InputClass::Invalid; // Specifies how input should be used.

		/* The number of instances to draw using the same per-instance
		 * data before advancing in the instance buffer by one element.
		 * (how many instances to draw per unique instance element) */
		uint32_t InstanceStepRate = 0;
	};
#pragma endregion

	using OpaquePtr = void*;

	enum class PLATFORM_API ShaderType : uint8_t
	{
		Invalid,
		Vertex,
		Pixel
	};

	using BufferPolicyUnderlying = uint8_t;
	using BufferTypeUnderlying = uint8_t;

	enum class PLATFORM_API BufferPolicy : BufferPolicyUnderlying
	{
		/* Represents a Buffer with GPU-Read/Write capabilities, but no CPU access.
		 * Can be used as both output and input to a stage. */
		Default = 0,

		/* Represents a Buffer with GPU-Read capabilities, and no CPU access. */
		Immutable = static_cast<BufferPolicyUnderlying>(1ull << 0),

		/* Represents a Buffer with GPU-Read and CPU-Write capabilities.
		 * Recommended for Buffer's that are updated every frame, or with high frequency. */
		Dynamic = static_cast<BufferPolicyUnderlying>(1ull << 1),

		/* Represents a Buffer with full GPU/CPU access.
		 * Is semantically used to copy data from the GPU to the CPU. */
		Staging = static_cast<BufferPolicyUnderlying>(1ull << 2),

		/* A basic flag indicating CPU-Write usage.
		 * Is only valid with a compatible basic usage (Dynamic, Staging). */
		Write = static_cast<BufferPolicyUnderlying>(1ull << ((sizeof(BufferPolicyUnderlying) * 8) - 1)),

		/* A basic flag indicating CPU-Read usage.
		 * Is only valid with a compatible basic usage (Staging). */
		Read = static_cast<BufferPolicyUnderlying>(1ull << ((sizeof(BufferPolicyUnderlying) * 8) - 1 - 1)),

		/* Represents a Dynamic Buffer with CPU-Write capabilities. */
		Dynamic_Write = Dynamic | Write,

		/* Represents a Staging Buffer with CPU-Read capabilities. */
		Staging_Read = Staging | Read,

		/* Represents a Staging Buffer with CPU-Write capabilities. */
		Staging_Write = Staging | Write,

		/* Represents a Staging Buffer with CPU-Read and CPU-Write capabilities. */
		Staging_ReadWrite = Staging | Read | Write,

		/* An alias for BufferPolicy::Default. See Default for remarks. */
		Unspecified = Default
	};

	enum class PLATFORM_API BufferType : BufferTypeUnderlying
	{
		Invalid = 0,
		Vertex,
		Index,
		Constant
	};

	PLATFORM_API inline constexpr [[nodiscard]] BufferPolicy operator|(BufferPolicy lhs, BufferPolicy rhs) noexcept;
	PLATFORM_API inline constexpr [[nodiscard]] BufferPolicy operator&(BufferPolicy lhs, BufferPolicy rhs) noexcept;
	PLATFORM_API inline constexpr BufferPolicy& operator|=(BufferPolicy& lhs, BufferPolicy rhs) noexcept;
	PLATFORM_API inline constexpr [[nodiscard]] BufferPolicyUnderlying EnumUnderlying(BufferPolicy policy) noexcept;

	/* Note: this is preferred over EnumUnderlying, as BufferPolicy::Default is currently
	 *         defined as 0, therefore policy & BufferPolicy::Default will always return 0. */
	PLATFORM_API inline constexpr [[nodiscard]] bool HasPolicy(BufferPolicy policy, BufferPolicy comparator) noexcept;

    /* Represents a cpu-side buffer of memory in bytes. */
    struct PLATFORM_API IBlob : public IRefCounted
    {
    public:
        IBlob() = default;
        virtual ~IBlob() = default;

        virtual [[nodiscard]] size_t Size() noexcept = 0;
        virtual [[nodiscard]] OpaquePtr Data() noexcept = 0;
    };

	struct PLATFORM_API IShader : public IRefCounted
	{
		IShader() = default;
		virtual ~IShader() = default;

		virtual [[nodiscard]] ShaderType Type() const noexcept = 0;
	};

	struct PLATFORM_API IVertexShader : public IShader
	{
		IVertexShader() = default;
		virtual ~IVertexShader() = default;

		virtual [[nodiscard]] ShaderType Type() const noexcept override final
		{
			return ShaderType::Vertex;
		}
    };

	struct PLATFORM_API IPixelShader : public IShader
	{
		IPixelShader() = default;
		virtual ~IPixelShader() = default;

		virtual [[nodiscard]] ShaderType Type() const noexcept override final
		{
			return ShaderType::Pixel;
		}
	};

    struct PLATFORM_API IInputLayout : public IRefCounted
    {
        IInputLayout() = default;
        virtual ~IInputLayout() = default;

        virtual [[nodiscard]] const InputElement* Elements() const noexcept = 0;
        virtual [[nodiscard]] size_t NumElements() const noexcept = 0;
    };

	struct PLATFORM_API IBuffer : public IRefCounted
	{
		IBuffer() = default;
		virtual ~IBuffer() = default;
	};

	struct PLATFORM_API IVertexBuffer : public IBuffer
	{
		IVertexBuffer() = default;
		virtual ~IVertexBuffer() = default;
	};

	struct PLATFORM_API IIndexBuffer : public IBuffer
	{
		IIndexBuffer() = default;
		virtual ~IIndexBuffer() = default;
	};

	struct PLATFORM_API IConstantBuffer : public IBuffer
	{
		IConstantBuffer() = default;
		virtual ~IConstantBuffer() = default;
	};

	PLATFORM_API inline constexpr [[nodiscard]] BufferPolicy operator|(BufferPolicy lhs, BufferPolicy rhs) noexcept
	{
		return static_cast<BufferPolicy>(
			static_cast<BufferPolicyUnderlying>(lhs) |
			static_cast<BufferPolicyUnderlying>(rhs)
		);
	}

	PLATFORM_API inline constexpr [[nodiscard]] BufferPolicy operator&(BufferPolicy lhs, BufferPolicy rhs) noexcept
	{
		return static_cast<BufferPolicy>(
			static_cast<BufferPolicyUnderlying>(lhs) &
			static_cast<BufferPolicyUnderlying>(rhs)
		);
	}

	PLATFORM_API inline constexpr BufferPolicy& operator|=(BufferPolicy& lhs, BufferPolicy rhs) noexcept
	{
		lhs = lhs | rhs;
		return lhs;
	}

	PLATFORM_API inline constexpr [[nodiscard]] BufferPolicyUnderlying EnumUnderlying(BufferPolicy policy) noexcept
	{
		return static_cast<BufferPolicyUnderlying>(policy);
	}

	PLATFORM_API inline constexpr [[nodiscard]] bool HasPolicy(BufferPolicy policy, BufferPolicy comparator) noexcept
	{
		if (policy == comparator)
			return true;
	
		return EnumUnderlying(policy & comparator);
	}
}