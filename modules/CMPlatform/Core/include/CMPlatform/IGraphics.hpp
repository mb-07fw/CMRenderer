#pragma once

#include <cstdint>

namespace CMEngine::Platform
{
	/* An RGBA value normalized to [-1, 1]. */
	struct RGBANorm
	{
		inline static constexpr [[nodiscard]] RGBANorm Red() noexcept { return { 1.0f, 0.0f, 0.0f, 1.0f }; }
		inline static constexpr [[nodiscard]] RGBANorm Green() noexcept { return { 1.0f, 0.0f, 0.0f, 1.0f }; }
		inline static constexpr [[nodiscard]] RGBANorm Blue() noexcept { return { 1.0f, 0.0f, 0.0f, 1.0f }; }
		inline static constexpr [[nodiscard]] RGBANorm Black() noexcept { return { 0.0f, 0.0f, 0.0f, 1.0f }; }
		inline static constexpr [[nodiscard]] RGBANorm White() noexcept { return { 1.0f, 1.0f, 1.0f, 1.0f }; }

		float rgba[4];
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
		using ClearSignature = void (*)(RGBANorm clearColor);
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
		void Clear(RGBANorm color) noexcept;
		void Present() noexcept;

		void Draw(const void* pBuffer, const DrawDescriptor* pDescriptor) noexcept;
	private:
		GraphicsFuncTable m_FuncTable;
	};
}