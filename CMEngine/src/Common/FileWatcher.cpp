#include "Core/PCH.hpp"
#include "Common/FileWatcher.hpp"
#include "Common/Utility.hpp"

namespace CMEngine::Common
{
	[[nodiscard]] WatcherID FileWatcher::WatchFile(
		const std::filesystem::path& filePath,
		std::function<void()> callbackFunc
	) noexcept
	{
		if (!std::filesystem::exists(filePath))
		{
			m_LastResult = WatcherResultType::FAILED_FILE_NOT_PRESENT;
			return WatcherID{};
		}
		else if (!std::filesystem::is_regular_file(filePath))
		{
			m_LastResult = WatcherResultType::FAILED_NON_FILE;
			return WatcherID{};
		}
		else if (!callbackFunc)
		{
			m_LastResult = WatcherResultType::FAILED_BAD_CALLBACK;
			return WatcherID{};
		}

		NativeStringView filePathView = filePath.c_str();

		NativeString fileNameString = filePath.filename().native();
		NativeString extensionString = filePath.extension().native();

		WatcherPathInfo info;
		info.ByteSize = filePathView.size() * sizeof(NativeChar);

		auto fileNamePos = filePathView.rfind(fileNameString);

		if (fileNamePos == std::string_view::npos)
		{
			m_LastResult = WatcherResultType::FAILED_FIND_FILENAME;
			return WatcherID{};
		}

		info.FilenameOffset = fileNamePos;

		if (!extensionString.empty())
		{
			auto extensionPos = filePathView.rfind(extensionString);

			if (extensionPos != std::string_view::npos &&
				extensionPos > info.FilenameOffset)
				info.ExtensionOffset = extensionPos;
			else
				info.HasExtension = false;
		}
		else
			info.HasExtension = false;

		m_PathArena.ReserveNew(info.ByteSize);
		
		if (!m_PathArena.Copy(Common::Utility::AsBytes(filePathView), info.ByteOffset))
		{
			m_LastResult = WatcherResultType::FAILED_COPY_PATH;
			return WatcherID{};
		}

		uint32_t nextID = static_cast<uint32_t>(m_InfoPool.size());

		auto test = std::filesystem::last_write_time(filePath);

		m_InfoPool.emplace_back(info);
		m_Callbacks.emplace_back(callbackFunc);

		m_LastResult = WatcherResultType::SUCCEEDED;
		return WatcherID(nextID);
	}
}