#pragma once

#include <array>
#include <cstdint>

namespace CMEngine::Platform
{
	/* NOTE: Unscoped-enum's are used for compatibility with `extern "C"` functions. */
	struct ShaderEnum
	{
		enum Enum : int8_t
		{
			INVALID = -1,
			VERTEX,
			PIXEL
		};
	};

	struct ActiveShaderEnum
	{
		enum Enum : int8_t
		{
			INVALID = -1,
			QUAD_VS,
			QUAD_PS
		};
	};

	struct ShaderSetEnum
	{
		enum Enum : int8_t
		{
			INVALID = -1,
			QUAD = 0,
			TOTAL_SETS
		};
	};

	/* An RGBA value normalized to [-1, 1]. */
	struct ColorNorm
	{
		float rgba[4];
	};

	inline constexpr std::array<ShaderSetEnum::Enum, static_cast<size_t>(ShaderSetEnum::TOTAL_SETS)> G_IMPLEMENTED_SHADER_SETS = {
		ShaderSetEnum::QUAD
	};

	struct DrawDescriptor
	{
		uint32_t TotalVertices = 0;
		uint32_t StartVertexLocation = 0; /* Index of the first vertex to draw. (Offset into the vertex buffer) */
		uint32_t VertexByteStride = 0;
	};

	struct GraphicsFuncTable
	{
		using VoidFunc = void (*)();
		using ClearSignature = void (*)(ColorNorm clearColor);
		using DrawSignature = void (*)(const void* pBuffer, const DrawDescriptor* descriptor);

		ClearSignature Clear = nullptr;
		VoidFunc Present = nullptr;
		DrawSignature Draw = nullptr;

		inline GraphicsFuncTable(
			ClearSignature clearFunc,
			VoidFunc presentFunc,
			DrawSignature drawFunc
		) noexcept
			: Clear(clearFunc),
			  Present(presentFunc),
			  Draw(drawFunc)
		{
		}
	};

	class IGraphics
	{
	public:
		IGraphics(const GraphicsFuncTable& funcTable) noexcept;
		virtual ~IGraphics() = default;
	public:
		void Clear(ColorNorm color) noexcept;
		void Present() noexcept;

		void Draw(const void* pBuffer, const DrawDescriptor* pDescriptor) noexcept;
	private:
		GraphicsFuncTable m_FuncTable;
	};
}