#pragma once

#include "Export.hpp"

#include <cstdint>

namespace CMEngine
{
	/* An RGBA value normalized to [-1, 1]. */
	struct CM_ENGINE_API RGBANorm
	{
		inline static constexpr [[nodiscard]] RGBANorm Red() noexcept { return { 1.0f, 0.0f, 0.0f, 1.0f }; }
		inline static constexpr [[nodiscard]] RGBANorm Green() noexcept { return { 1.0f, 0.0f, 0.0f, 1.0f }; }
		inline static constexpr [[nodiscard]] RGBANorm Blue() noexcept { return { 1.0f, 0.0f, 0.0f, 1.0f }; }
		inline static constexpr [[nodiscard]] RGBANorm Black() noexcept { return { 0.0f, 0.0f, 0.0f, 1.0f }; }
		inline static constexpr [[nodiscard]] RGBANorm White() noexcept { return { 1.0f, 1.0f, 1.0f, 1.0f }; }

		inline constexpr [[nodiscard]] float r() const noexcept { return rgba[0]; }
		inline constexpr [[nodiscard]] float g() const noexcept { return rgba[1]; }
		inline constexpr [[nodiscard]] float b() const noexcept { return rgba[2]; }
		inline constexpr [[nodiscard]] float a() const noexcept { return rgba[3]; }

		float rgba[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	};

	struct CM_ENGINE_API DrawDescriptor
	{
		uint32_t TotalVertices = 0;
		uint32_t StartVertexLocation = 0; /* Index of the first vertex to draw. (Offset into the vertex buffer) */
		uint32_t VertexByteStride = 0;
	};

	class CM_ENGINE_API IGraphics
	{
	public:
		IGraphics() = default;
		virtual ~IGraphics() = default;
	public:
		virtual void Clear(RGBANorm color) noexcept = 0;
		virtual void Present() noexcept = 0;

		virtual void Draw(const void* pBuffer, const DrawDescriptor& pDescriptor) noexcept = 0;
	};
}