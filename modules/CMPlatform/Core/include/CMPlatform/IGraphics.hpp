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

	struct GraphicsFuncTable
	{
		using ColorFuncSignature = void (*)(ColorNorm color);
		using VoidFunc = void (*)();

		ColorFuncSignature Clear = nullptr;
		VoidFunc Present = nullptr;

		inline GraphicsFuncTable(
			ColorFuncSignature clearFunc,
			VoidFunc presentFunc
		) noexcept
			: Clear(clearFunc),
			  Present(presentFunc)
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
	private:
		GraphicsFuncTable m_FuncTable;
	};
}