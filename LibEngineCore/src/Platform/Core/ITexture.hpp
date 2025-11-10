#pragma once

#include "Platform/Core/IUploadable.hpp"

namespace CMEngine
{
	class ITexture : public IUploadable
	{
	public:
		ITexture() = default;
		~ITexture() = default;
	};
}