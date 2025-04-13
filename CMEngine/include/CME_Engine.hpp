#pragma once

#include "CMR_Renderer.hpp"

namespace CMEngine
{
	class CMEngine
	{
	public:
		CMEngine() noexcept;
		~CMEngine() noexcept;
	public:
		void Run() noexcept;
	private:
		CMRenderer::CMRenderer m_Renderer;
	};
}