#pragma once

#include <cstdint>

#include "Core_Export.hpp"

namespace CMPlatform
{
	/* An RGBA value normalized to [-1, 1]. */
	struct CM_PLATFORM_API RGBANorm
	{
		inline static constexpr [[nodiscard]] RGBANorm Red() noexcept { return { 1.0f, 0.0f, 0.0f, 1.0f }; }
		inline static constexpr [[nodiscard]] RGBANorm Green() noexcept { return { 1.0f, 0.0f, 0.0f, 1.0f }; }
		inline static constexpr [[nodiscard]] RGBANorm Blue() noexcept { return { 1.0f, 0.0f, 0.0f, 1.0f }; }
		inline static constexpr [[nodiscard]] RGBANorm Black() noexcept { return { 0.0f, 0.0f, 0.0f, 1.0f }; }
		inline static constexpr [[nodiscard]] RGBANorm White() noexcept { return { 1.0f, 1.0f, 1.0f, 1.0f }; }

		float rgba[4];
	};

	struct CM_PLATFORM_API DrawDescriptor
	{
		uint32_t TotalVertices = 0;
		uint32_t StartVertexLocation = 0; /* Index of the first vertex to draw. (Offset into the vertex buffer) */
		uint32_t VertexByteStride = 0;
	};

	class CM_PLATFORM_API IGraphics
	{
	public:
		IGraphics() noexcept;
		virtual ~IGraphics() = default;
	public:
		virtual void Clear(RGBANorm color) noexcept = 0;
		virtual void Present() noexcept = 0;

		virtual void Draw(const void* pBuffer, const DrawDescriptor* pDescriptor) noexcept = 0;
	};
}