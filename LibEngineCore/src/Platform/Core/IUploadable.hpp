#pragma once

#include <cstdint>

namespace CMEngine
{
	/* A pure virtual interface for an object that is bindable to the graphics pipeline. */
	class IUploadable
	{
	public:
		IUploadable() = default;
		virtual ~IUploadable() = default;
	};

	class IBuffer : public IUploadable
	{
	public:
		IBuffer() = default;
		virtual ~IBuffer() = default;
	};

	using GPUBufferFlagUnderlying = uint8_t;
	using GPUBufferTypeUnderlying = int8_t;

	enum class GPUBufferFlag : GPUBufferFlagUnderlying
	{
		/* Represents a GPUBuffer with GPU-Read/Write capabilities, but no CPU access. 
		 * Can be used as both output and input to a stage. */
		Default = 0,

		/* Represents a GPUBuffer with GPU-Read capabilities, and no CPU access. */
		Immutable = static_cast<GPUBufferFlagUnderlying>(1ull << 0),

		/* Represents a GPUBuffer with GPU-Read and CPU-Write capabilities.
		 * Recommended for GPUBuffer's that are updated every frame, or with high frequency. */
		Dynamic = static_cast<GPUBufferFlagUnderlying>(1ull << 1),
		
		/* Represents a GPUBuffer with full GPU/CPU access.
		 * Is semantically used to copy data from the GPU to the CPU. */
		Staging = static_cast<GPUBufferFlagUnderlying>(1ull << 2),

		/* A basic flag indicating CPU-Write usage. Is only valid with a compatible basic usage (Dynamic, Staging). */
		Write = static_cast<GPUBufferFlagUnderlying>(1ull << ((sizeof(GPUBufferFlagUnderlying) * 8) - 1)),

		/* A basic flag indicating CPU-Read usage. Is only valid with a compatible basic usage (Staging). */
		Read = static_cast<GPUBufferFlagUnderlying>(1ull << ((sizeof(GPUBufferFlagUnderlying) * 8) - 1 - 1)),

		/* Represents a Dynamic GPUBuffer with CPU-Write capabilities. */
		Dynamic_Write = Dynamic | Write,

		/* Represents a Staging GPUBuffer with CPU-Read capabilities. */
		Staging_Read = Staging | Read,

		/* Represents a Staging GPUBuffer with CPU-Write capabilities. */
		Staging_Write = Staging | Write,

		/* Represents a Staging GPUBuffer with CPU-Read and CPU-Write capabilities. */
		Staging_ReadWrite = Staging | Read | Write,

		/* An alias for GPUBufferFlag::Default. See Default for remarks. */
		Unspecified = Default
	};

	enum class GPUBufferType : GPUBufferTypeUnderlying
	{
		Invalid = -1,
		Vertex,
		Index,
		Constant
	};

	inline constexpr GPUBufferFlag operator|(GPUBufferFlag a, GPUBufferFlag b) noexcept
	{
		return static_cast<GPUBufferFlag>(
			static_cast<GPUBufferFlagUnderlying>(a) |
			static_cast<GPUBufferFlagUnderlying>(b)
			);
	}

	inline constexpr GPUBufferFlag operator&(GPUBufferFlag a, GPUBufferFlag b) noexcept
	{
		return static_cast<GPUBufferFlag>(
			static_cast<GPUBufferFlagUnderlying>(a) &
			static_cast<GPUBufferFlagUnderlying>(b)
			);
	}

	inline constexpr GPUBufferFlag& operator|=(GPUBufferFlag& a, GPUBufferFlag b) noexcept
	{
		a = a | b;
		return a;
	}

	inline constexpr [[nodiscard]] GPUBufferFlagUnderlying FlagUnderlying(GPUBufferFlag flag)
	{
		return static_cast<GPUBufferFlagUnderlying>(flag);
	}
}