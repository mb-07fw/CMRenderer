#include "CME_Engine.hpp"
#include "CMC_Paths.hpp"
#include "CMC_Utility.hpp"

/*
 *  TODO in order :
 *		1. Refactor shader sets to have concrete shader sets that are specialized. (e.g., CMRectShaderSet, CMInterpRectShaderSet)
 *      2. Write a system to translate CMRect to an actual rectangle on screen.
 *		3. Finish the shape queueing system.
 *	Future :
 *		Add instanced renderering, vertex array specialization, etc.
 * 
 */

namespace CMEngine
{
	CMEngine::CMEngine() noexcept
		: m_Renderer(CMRenderer::CMWindowData(L"Title", false, 0, 0))
	{
		m_Renderer.Init();
	}

	void CMEngine::Run() noexcept
	{
		/*m_Renderer.QueueShape(CMCommon::CMFloat2{  5, 10 });
		m_Renderer.QueueShape(CMCommon::CMFloat2{ -4,  5 });*/

		CMCommon::CMRect rect = { { 0.0f, 0.0f } };

		while (m_Renderer.Window().IsRunning())
		{
			m_Renderer.UpdateWindow();

			m_Renderer.Clear(CMCommon::NormColor{ 0.1f, 0.1f, 0.1f, 1.0f });

			m_Renderer.DrawRect(rect);

			m_Renderer.Present();
		}
	}

	CMEngine::~CMEngine() noexcept
	{
		m_Renderer.Shutdown();
	}
}