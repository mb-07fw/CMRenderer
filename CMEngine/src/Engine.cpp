#include "Engine.hpp"

#include <chrono>
#include <thread>

namespace CMEngine
{
	Engine::Engine() noexcept
	{
	}

	void Engine::Run() noexcept
	{
		/* TODO:
		 *   1 - Implement depth testing. 
		 * 
		 */

		double deltaTime = 0.0f;
		while (m_Platform.IsRunning())
		{
			auto startTime = std::chrono::high_resolution_clock::now();

			m_Platform.Update();

			auto endTime = std::chrono::high_resolution_clock::now();

			deltaTime = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count());

			constexpr double TargetFrameTime = 1000.0f / 60.0f;

			std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(TargetFrameTime - deltaTime));
		}
	}
}