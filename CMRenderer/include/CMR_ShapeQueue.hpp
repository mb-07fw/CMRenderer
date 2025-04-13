#pragma once

#include "CMC_Shapes.hpp"
#include "CMC_MetaArena.hpp"

namespace CMRenderer
{
	class CMShapeQueue
	{
	public:
		CMShapeQueue() noexcept;
		~CMShapeQueue() noexcept;
	public:
		void Queue(CMCommon::CMRect rect) noexcept;
	private:
		static constexpr size_t S_QUEUE_TOTAL_BYTE_SIZE = 1024 * 5; // 5 kb.
		static constexpr size_t S_TOTAL_INITIAL_ALLOCDATA = 30;
		CMCommon::CMMetaArena<CMCommon::CMShapeType> m_Allocator;
	};
}