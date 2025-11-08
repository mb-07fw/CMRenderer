#pragma once

#include "Platform/Core/IUploadable.hpp"
#include "Platform/Core/InputElement.hpp"

#include <vector>

namespace CMEngine
{
	class IInputLayout : public IUploadable
	{
	public:
		IInputLayout() = default;
		virtual ~IInputLayout() = default;
	};

	class InputLayoutBase : public IInputLayout
	{
	public:
		inline InputLayoutBase(std::span<const InputElement> elements) noexcept
		{
			m_Elements.reserve(elements.size());
			m_Elements.assign(elements.begin(), elements.end());
		}

		virtual ~InputLayoutBase() = default;

		inline [[nodiscard]] const std::vector<InputElement>& Elements() const noexcept { return m_Elements; }
	protected:
		std::vector<InputElement> m_Elements;
	};
}