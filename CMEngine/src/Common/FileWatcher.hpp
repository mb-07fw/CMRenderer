#ifndef _WIN32
	#error This file must be built on Windows due to it's use of the WinAPI.
#endif

#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <filesystem>
#include <type_traits>
#include <functional>

#include "Common/ErasedArena.hpp"

namespace CMEngine::Common
{
	enum class WatcherResultType : int8_t
	{
		INVALID = -127,
		FAILED_FILE_NOT_PRESENT,
		FAILED_NON_FILE,
		FAILED_BAD_CALLBACK,
		FAILED_FIND_FILENAME,
		FAILED_COPY_PATH,
		FAILED = 0,
		SUCCEEDED,
	};

	struct WatcherResult
	{
		WatcherResultType Type = WatcherResultType::INVALID;

		inline constexpr WatcherResult(WatcherResultType type) noexcept;
		inline constexpr WatcherResult() = default;
		inline constexpr ~WatcherResult() = default;

		inline static constexpr [[nodiscard]] int8_t ToInt8(WatcherResultType type) noexcept { return static_cast<int8_t>(type); }

		inline constexpr [[nodiscard]] bool Succeeded() const noexcept { return ToInt8(Type) > ToInt8(WatcherResultType::FAILED); }
		inline constexpr [[nodiscard]] bool Failed() const noexcept { return ToInt8(Type) < ToInt8(WatcherResultType::SUCCEEDED); }

		inline constexpr operator bool() noexcept { return Succeeded(); }
		inline constexpr [[nodiscard]] bool operator!() noexcept { return Failed(); }
		inline constexpr [[nodiscard]] bool operator==(WatcherResult other) noexcept { return Type == other.Type; }
		inline WatcherResult& operator=(WatcherResultType type) noexcept { Type = type; return *this; }

		inline static constexpr [[nodiscard]] bool IsSucceeded(WatcherResult result) noexcept { return result.Succeeded(); }
		inline static constexpr [[nodiscard]] bool IsFailed(WatcherResult result) noexcept { return result.Failed(); }
	};

	struct WatcherID
	{
		static constexpr uint32_t S_INVALID_ID = std::numeric_limits<uint32_t>::max();
		uint32_t ID = S_INVALID_ID;

		inline constexpr WatcherID(uint32_t id) noexcept;
		inline constexpr WatcherID() = default;
		inline constexpr ~WatcherID() = default;

		inline constexpr [[nodiscard]] bool Valid() const noexcept { return ID != S_INVALID_ID; }
		inline constexpr [[nodiscard]] bool Invalid() const noexcept { return ID == S_INVALID_ID; }
		inline operator bool() const noexcept { return Valid(); }
		inline [[nodiscard]] bool operator!() const noexcept { return Invalid(); }
	};

	struct WatcherPathInfo
	{
		size_t ByteSize = 0; /* Byte size of the file path. */
		size_t ByteOffset = 0; /* Byte offset into Path Arena. */
		size_t FilenameOffset = 0; /* Character offset from the start of the path to the file's name. */
		size_t ExtensionOffset = 0; /* Character offset from the start of the path to the file's extension. */
		bool HasExtension = true;
	};

	template <typename Ty>
	concept IsWideString = std::is_same_v<Ty, std::wstring>;

	using NativeString = std::filesystem::path::string_type;

	using NativeStringView = std::conditional_t<
		IsWideString<NativeString>,
		std::wstring_view,
		std::string_view>;

	using NativeChar = std::conditional_t<
		IsWideString<NativeString>,
		wchar_t,
		char>;

	class File
	{
	private:
		WatcherPathInfo PathInfo;
		NativeStringView Path;
	};

	class FileWatcher
	{
	public:
		
	public:
		FileWatcher() = default;
		~FileWatcher() = default;

		FileWatcher(const FileWatcher& other) = delete;
		FileWatcher& operator=(const FileWatcher& other) = delete;
	public:
		[[nodiscard]] WatcherID WatchFile(
			const std::filesystem::path& filePath,
			std::function<void()> callbackFunc
		) noexcept;

		inline [[nodiscard]] WatcherResult LastResult() const noexcept { return m_LastResult; }
	private:
		ErasedArena m_PathArena;
		WatcherResult m_LastResult;
		std::vector<WatcherPathInfo> m_InfoPool; /* Indexed into by each WatcherID. (Should be parallel to all other vector's) */
		std::vector<std::function<void()>> m_Callbacks; /* Indexed into by each WatcherID. (Should be parallel to all other vector's) */
	};

	inline constexpr WatcherResult::WatcherResult(WatcherResultType type) noexcept
		: Type(type)
	{
	}

	inline constexpr WatcherID::WatcherID(uint32_t id) noexcept
		: ID(id)
	{
	}
}