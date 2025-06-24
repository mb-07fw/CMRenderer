#pragma once

#include "CMC_ECSEntity.hpp"

namespace CMEngine
{
	struct CMEntity
	{
		CMCommon::CMECSEntity Entity;
	};

	struct CMCameraEntity : public CMEntity
	{
	};
}