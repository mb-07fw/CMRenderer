#pragma once

#include "CMC_Logger.hpp"

namespace CMEngine
{
	class CMLayerStack
	{
	public:
		CMLayerStack(CMCommon::CMLoggerWide& logger) noexcept;
		~CMLayerStack() = default;
	private:
		CMCommon::CMLoggerWide& m_Logger;
	};
}