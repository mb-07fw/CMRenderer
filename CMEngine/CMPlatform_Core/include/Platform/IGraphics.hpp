#pragma once

namespace CMEngine::Platform
{
	class IGraphics
	{
	public:
		IGraphics() = default;
		virtual ~IGraphics() = default;
	private:
		// It's own function table later...
	};
}