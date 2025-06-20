#include "CMC_BinaryStream.hpp"

namespace CMCommon
{
	CMBinaryStreamInput::CMBinaryStreamInput(const std::filesystem::path& filePathRef) noexcept
		: Parent(CMCommon::CMBinaryStreamType::READ, filePathRef)
	{
	}

	CMBinaryStreamOutput::CMBinaryStreamOutput(const std::filesystem::path& filePathRef) noexcept
		: Parent(CMCommon::CMBinaryStreamType::WRITE, filePathRef)
	{
	}

	CMBinaryStreamIO::CMBinaryStreamIO(const std::filesystem::path& filePathRef) noexcept
		: Parent(CMCommon::CMBinaryStreamType::READ_WRITE, filePathRef)
	{
	}
}