#include "CMR_PCH.hpp"
#include "CMR_ShapeQueue.hpp"

namespace CMRenderer
{
	CMShapeQueue::CMShapeQueue() noexcept
		: m_Allocator(S_QUEUE_TOTAL_BYTE_SIZE, S_TOTAL_INITIAL_ALLOCDATA)
	{
	}

	CMShapeQueue::~CMShapeQueue() noexcept
	{
	}

	void CMShapeQueue::Queue(CMCommon::CMRect rect) noexcept
	{
		m_Allocator.ConstructNext<CMCommon::CMRect>(CMCommon::CMShapeType::RECT, rect);
	}
}