#include "Common/BinaryStream.hpp"

namespace CMEngine::Common
{
	BinaryStreamInput::BinaryStreamInput(const std::filesystem::path& filePathRef) noexcept
		: Parent(Common::BinaryStreamType::READ, filePathRef)
	{
	}

	BinaryStreamOutput::BinaryStreamOutput(const std::filesystem::path& filePathRef) noexcept
		: Parent(Common::BinaryStreamType::WRITE, filePathRef)
	{
	}

	BinaryStreamIO::BinaryStreamIO(const std::filesystem::path& filePathRef) noexcept
		: Parent(Common::BinaryStreamType::READ_WRITE, filePathRef)
	{
	}
}