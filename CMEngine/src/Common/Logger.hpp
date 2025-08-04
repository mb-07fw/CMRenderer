#pragma once

#include <iostream>
#include <fstream>
#include <string_view>
#include <type_traits>
#include <filesystem>
#include <format>

#include "Common/Macros.hpp"

/* Logger :
 *
 *		A fail-fast type logger where fatal logs == program termination.
 *      I spent too much time writing this to not be biased, but this is clearly
 *		the best logger ever written.
 *
 *	-- mb-07fw.
 *
 */

namespace CMEngine::Common
{
	template <typename Ty>
	struct IsLoggerViewType : std::bool_constant<
		std::is_same_v<Ty, std::string_view> ||
		std::is_same_v<Ty, std::wstring_view>> {
	};

	template <typename Ty>
	concept LoggerViewType = IsLoggerViewType<Ty>::value;

	template <typename Ty>
	struct IsLoggerDataType : std::bool_constant<
		std::is_same_v<Ty, std::string> ||
		std::is_same_v<Ty, std::wstring>> {
	};

	template <typename Ty>
	concept LoggerDataType = IsLoggerDataType<Ty>::value;

	template <typename Ty>
	concept LoggableType = LoggerViewType<Ty> || LoggerDataType<Ty>;

	template <typename Ty>
	struct IsValidStream : std::bool_constant<
		std::is_same_v<Ty, std::ofstream> ||
		std::is_same_v<Ty, std::wofstream>> {
	};

	template <typename Ty>
	concept ValidStream = IsValidStream<Ty>::value;

	enum class LoggerState
	{
		INVALID = -1, 
		TARGETED,
		UNTARGETED
	};

	enum class LoggerType
	{
		CM_WIDE_LOGGER,
		CM_NARROW_LOGGER
	};

	#define LoggerTmplSignature template <LoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy> \
		requires LoggerDataType<DataTy>&& LoggerViewType<ViewTy>&& ValidStream<StreamTy>

	#define LoggerTmpl Logger<EnumLoggerType, DataTy, ViewTy, StreamTy>

	LoggerTmplSignature
	class Logger
	{
	public:
		inline Logger(ViewTy targetFileName) noexcept;
		inline Logger() = default;
		inline ~Logger() noexcept;
	public:
		inline void LogInfo(ViewTy data) noexcept;
		inline void LogInfoNL(ViewTy data) noexcept;
		inline void LogInfoNLTagged(ViewTy tag, ViewTy data) noexcept;
		inline bool LogInfoNLTaggedIf(bool condition, ViewTy tag, ViewTy data) noexcept;

		template <typename AppendTy>
		inline void LogInfoNLAppend(ViewTy data, AppendTy append) noexcept;

		template <typename... Args>
		inline void LogInfoNLVariadic(ViewTy data, Args&&... args) noexcept;

		template <typename... Args>
		inline void LogInfoNLFormatted(ViewTy originTag, ViewTy fmt, Args&&... args) noexcept;

		template <typename... Args>
		inline bool LogInfoNLFormattedIf(bool condition, ViewTy originTag, ViewTy fmt, Args&&... args) noexcept;

		inline void LogWarning(ViewTy data) noexcept;
		inline void LogWarningNL(ViewTy data) noexcept;
		inline bool LogWarningNLIf(bool condition, ViewTy data) noexcept;
		inline void LogWarningNLTagged(ViewTy tag, ViewTy data) noexcept;
		inline bool LogWarningNLTaggedIf(bool condition, ViewTy tag, ViewTy data) noexcept;

		template <typename AppendTy>
		inline void LogWarningNLAppend(ViewTy data, AppendTy append) noexcept;

		template <typename AppendTy>
		inline bool LogWarningNLAppendIf(bool condition, ViewTy data, AppendTy appendData) noexcept;

		template <typename... Args>
		inline void LogWarningNLVariadic(ViewTy data, Args&&... args) noexcept;

		template <typename... Args>
		inline bool LogWarningNLVariadicIf(bool condition, ViewTy data, Args&&... args) noexcept;

		template <typename... Args>
		inline void LogWarningNLFormatted(ViewTy originTag, ViewTy fmt, Args&&... args) noexcept;

		template <typename... Args>
		inline bool LogWarningNLFormattedIf(bool condition, ViewTy originTag, ViewTy fmt, Args&&... args) noexcept;

		inline void LogFatal(ViewTy data, int exitCode = -1) noexcept;
		inline void LogFatalTagged(ViewTy tag, ViewTy data, int exitCode = -1) noexcept;
		inline void LogFatalNLTagged(ViewTy tag, ViewTy data, int exitCode = -1) noexcept;
		inline void LogFatalNLTaggedIf(bool condition, ViewTy tag, ViewTy data, int exitCode = -1) noexcept;
		inline void LogFatalNL(ViewTy data, int exitCode = -1) noexcept;
		inline void LogFatalNLIf(bool condition, ViewTy data, int exitCode = -1) noexcept;

		template <typename AppendTy>
		inline void LogFatalNLAppend(ViewTy data, AppendTy append, int exitCode = -1) noexcept;

		template <typename AppendTy>
		inline void LogFatalNLAppendIf(bool condition, ViewTy data, AppendTy appendData, int exitCode = -1) noexcept;

		template <typename... Args>
		inline void LogFatalNLVariadic(int exitCode, ViewTy data, Args&&... args) noexcept;

		template <typename... Args>
		inline void LogFatalNLVariadicIf(bool condition, ViewTy data, Args&&... args) noexcept;

		template <typename... Args>
		inline void LogFatalNLFormatted(ViewTy tag, ViewTy fmt, Args&&... args) noexcept;

		template <typename... Args>
		inline void LogFatalNLFormattedIf(bool condition, ViewTy originTag, ViewTy fmt, Args&&... args) noexcept;

		inline void LogInline(ViewTy data) noexcept;

		inline bool OpenFile(ViewTy fileName) noexcept;
		inline bool OpenFileInDirectory(ViewTy fileName, const std::filesystem::path& directory) noexcept;
		inline void CloseFile() noexcept;

		inline [[nodiscard]] bool IsStreamOpen() const noexcept { return m_LogStream.is_open(); }
		inline [[nodiscard]] const DataTy& TargetFileName() const noexcept { return m_TargetFileName; }
	private:
		inline void LogFlag(ViewTy data, ViewTy flag) noexcept;
		inline void LogFlagTagged(ViewTy data, ViewTy flag, ViewTy tag) noexcept;
		inline void LogFlagNL(ViewTy data, ViewTy flag) noexcept;
		inline void LogFlagTaggedNL(ViewTy data, ViewTy flag, ViewTy tag) noexcept;

		template <typename AppendTy>
		inline void LogFlagNLAppend(ViewTy data, AppendTy append, ViewTy flag) noexcept;

		template <typename... Args>
		inline void LogFlagNLVariadic(ViewTy data, ViewTy flag, Args&&... args) noexcept;
	private:
		static constexpr bool S_IS_WIDE_LOGGER = std::is_same_v<ViewTy, std::wstring_view>;
		static constexpr ViewTy S_LOG_TAG = [] {
			if constexpr (S_IS_WIDE_LOGGER)
				return ViewTy(L"LoggerWide ");
			else
				return ViewTy("Logger ");
		}();
	private:
		DataTy m_TargetFileName;
		StreamTy m_LogStream;
	};

#pragma region Logger
#pragma region Public
	LoggerTmplSignature
	inline LoggerTmpl::Logger(ViewTy targetFileName) noexcept
		: m_LogStream(targetFileName.data()),
		  m_TargetFileName(targetFileName.data())
	{
	}

	LoggerTmplSignature
	inline LoggerTmpl::~Logger() noexcept
	{
		CloseFile();
	}

#pragma region LogInfo
	LoggerTmplSignature
	inline void LoggerTmpl::LogInfo(ViewTy data) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
			LogFlag(data, L"INFO");
		else
			LogFlag(data, "INFO");
	}

	LoggerTmplSignature
	inline void LoggerTmpl::LogInfoNL(ViewTy data) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
			LogFlagNL(data, L"INFO");
		else
			LogFlagNL(data, "INFO");
	}

	LoggerTmplSignature
	inline void LoggerTmpl::LogInfoNLTagged(ViewTy tag, ViewTy data) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
			LogFlagTaggedNL(data, L"WARNING", tag);
		else
			LogFlagTaggedNL(data, "WARNING", tag);
	}

	LoggerTmplSignature
	inline bool LoggerTmpl::LogInfoNLTaggedIf(bool condition, ViewTy tag, ViewTy data) noexcept
	{
		if (!condition)
			return false;

		LogInfoNLTagged(tag, data);
		return true;
	}

	LoggerTmplSignature
	template <typename AppendTy>
	inline void LoggerTmpl::LogInfoNLAppend(ViewTy data, AppendTy append) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
			LogFlagNLAppend(data, append, L"INFO");
		else
			LogFlagNLAppend(data, append, "INFO");
	}

	LoggerTmplSignature
	template <typename... Args>
	inline void LoggerTmpl::LogInfoNLVariadic(ViewTy data, Args&&... args) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
			LogFlagNLVariadic(data, L"INFO", std::forward<Args>(args)...);
		else
			LogFlagNLVariadic(data, "INFO", std::forward<Args>(args)...);
	}

	LoggerTmplSignature
	template <typename... Args>
	inline void LoggerTmpl::LogInfoNLFormatted(ViewTy originTag, ViewTy fmt, Args&&... args) noexcept
	{
		DataTy formatted = std::vformat(fmt, std::make_wformat_args(std::forward<Args>(args)...));

		LogInfoNLTagged(originTag, formatted);
	}

	LoggerTmplSignature
	template <typename... Args>
	inline bool LoggerTmpl::LogInfoNLFormattedIf(bool condition, ViewTy originTag, ViewTy fmt, Args&&... args) noexcept
	{
		if (!condition)
			return false;

		LogInfoNLFormatted(originTag, fmt, std::forward<Args>(args)...);
		return true;
	}
#pragma endregion

#pragma region LogWarning
	LoggerTmplSignature
	inline void LoggerTmpl::LogWarning(ViewTy data) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
			LogFlag(data, L"WARNING");
		else
			LogFlag(data, "WARNING");
	}

	LoggerTmplSignature
	inline void LoggerTmpl::LogWarningNL(ViewTy data) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
			LogFlagNL(data, L"WARNING");
		else
			LogFlagNL(data, "WARNING");
	}

	LoggerTmplSignature
	inline bool LoggerTmpl::LogWarningNLIf(bool condition, ViewTy data) noexcept
	{
		if (!condition)
			return false;

		LogWarningNL(data);
		return true;
	}

	LoggerTmplSignature
	inline void LoggerTmpl::LogWarningNLTagged(ViewTy tag, ViewTy data) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
			LogFlagTaggedNL(data, L"WARNING", tag);
		else
			LogFlagTaggedNL(data, "WARNING", tag);
	}

	LoggerTmplSignature
	inline bool LoggerTmpl::LogWarningNLTaggedIf(bool condition, ViewTy tag, ViewTy data) noexcept
	{
		if (!condition)
			return false;

		LogWarningNLTagged(tag, data);
		return true;
	}

	LoggerTmplSignature
	template <typename AppendTy>
	inline void LoggerTmpl::LogWarningNLAppend(ViewTy data, AppendTy append) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
			LogFlagNLAppend(data, append, L"WARNING");
		else
			LogFlagNLAppend(data, append, "WARNING");
	}

	LoggerTmplSignature
	template <typename AppendTy>
	inline bool LoggerTmpl::LogWarningNLAppendIf(bool condition, ViewTy data, AppendTy append) noexcept
	{
		if (!condition)
			return false;

		LogWarningNLAppend(data, append);
		return true;
	}

	LoggerTmplSignature
	template <typename... Args>
	inline void LoggerTmpl::LogWarningNLVariadic(ViewTy data, Args&&... args) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
			LogFlagNLVariadic(data, L"WARNING", std::forward<Args>(args)...);
		else
			LogFlagNLVariadic(data, "WARNING", std::forward<Args>(args)...);
	}

	LoggerTmplSignature
	template <typename... Args>
	inline bool LoggerTmpl::LogWarningNLVariadicIf(bool condition, ViewTy data, Args&&... args) noexcept
	{
		if (!condition)
			return false;

		LogWarningNLVariadic(data, std::forward<Args>(args)...);
		return true;
	}

	LoggerTmplSignature
	template <typename... Args>
	inline void LoggerTmpl::LogWarningNLFormatted(ViewTy originTag, ViewTy fmt, Args&&... args) noexcept
	{
		DataTy formatted = std::vformat(fmt, std::make_wformat_args(std::forward<Args>(args)...));

		LogWarningNLTagged(originTag, formatted);
	}

	LoggerTmplSignature
	template <typename... Args>
	inline bool LoggerTmpl::LogWarningNLFormattedIf(bool condition, ViewTy originTag, ViewTy fmt, Args&&... args) noexcept
	{
		if (!condition)
			return false;

		LogWarningNLFormatted(originTag, fmt, std::forward<Args>(args)...);
		return true;
	}
#pragma endregion

#pragma region LogFatal
	LoggerTmplSignature
	inline void LoggerTmpl::LogFatal(ViewTy data, int exitCode) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
		{
			LogFlag(data, L"FATAL");
			LogFlagNLAppend(
				L"Logger [LogFatal] | Logger is terminating the program with the exit code : ",
				exitCode,
				L"FATAL"
			);
		}
		else
		{
			LogFlag(data, "FATAL");

			LogFlagNLAppend(
				"Logger [LogFatal] | Logger is terminating the program with the exit code : ",
				exitCode,
				"FATAL"
			);
		}

		CM_BREAK_DEBUGGER();
		exit(exitCode);
	}

	LoggerTmplSignature
	inline void LoggerTmpl::LogFatalTagged(ViewTy tag, ViewTy data, int exitCode) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
		{
			LogFlagTagged(data, L"FATAL", tag);
			LogFlagNLAppend(
				L"Logger [LogFatal] | Logger is terminating the program with the exit code : ",
				exitCode,
				L"FATAL"
			);
		}
		else
		{
			LogFlagTagged(data, "FATAL", tag);

			LogFlagNLAppend(
				"Logger [LogFatal] | Logger is terminating the program with the exit code : ",
				exitCode,
				"FATAL"
			);
		}

		CM_BREAK_DEBUGGER();
		exit(exitCode);
	}

	LoggerTmplSignature
	inline void LoggerTmpl::LogFatalNLTagged(ViewTy tag, ViewTy data, int exitCode) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
		{
			LogFlagTaggedNL(data, L"FATAL", tag);

			LogFlagNLAppend(
				L"Logger [LogFatalNL] | Logger is terminating the program with the exit code : ",
				exitCode,
				L"FATAL"
			);
		}
		else
		{
			LogFlagTaggedNL(data, "FATAL", tag);

			LogFlagNLAppend(
				"Logger [LogFatalNL] | Logger is terminating the program with the exit code : ",
				exitCode,
				"FATAL"
			);
		}

		CM_BREAK_DEBUGGER();
		exit(exitCode);
	}

	LoggerTmplSignature
	inline void LoggerTmpl::LogFatalNLTaggedIf(bool condition, ViewTy tag, ViewTy data, int exitCode) noexcept
	{
		if (condition)
			LogFatalNLTagged(tag, data, exitCode);
	}

	LoggerTmplSignature
	inline void LoggerTmpl::LogFatalNL(ViewTy data, int exitCode) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
		{
			LogFlagNL(data, L"FATAL");

			LogFlagNLAppend(
				L"Logger [LogFatalNL] | Logger is terminating the program with the exit code : ",
				exitCode,
				L"FATAL"
			);
		}
		else
		{
			LogFlagNL(data, "FATAL");

			LogFlagNLAppend(
				"Logger [LogFatalNL] | Logger is terminating the program with the exit code : ",
				exitCode,
				"FATAL"
			);
		}

		CM_BREAK_DEBUGGER();
		exit(exitCode);
	}

	LoggerTmplSignature
	inline void LoggerTmpl::LogFatalNLIf(bool condition, ViewTy data, int exitCode) noexcept
	{
		if (condition)
			LogFatalNL(data, exitCode);
	}

	LoggerTmplSignature
	template <typename AppendTy>
	inline void LoggerTmpl::LogFatalNLAppend(ViewTy data, AppendTy append, int exitCode) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
		{
			LogFlagNLAppend(data, append, L"FATAL");

			LogFlagNLAppend(
				L"Logger [LogFatalNLAppend] | Logger is terminating the program with the exit code : ",
				exitCode,
				L"FATAL"
			);
		}
		else
		{
			LogFlagNLAppend(data, append, "FATAL");

			LogFlagNLAppend(
				"Logger [LogFatalNLAppend] | Logger is terminating the program with the exit code : ",
				exitCode,
				"FATAL"
			);
		}

		CM_BREAK_DEBUGGER();
		exit(exitCode);
	}

	LoggerTmplSignature
	template <typename AppendTy>
	inline void LoggerTmpl::LogFatalNLAppendIf(bool condition, ViewTy data, AppendTy append, int exitCode) noexcept
	{
		if (condition)
			LogFatalNLAppend(data, append, exitCode);
	}

	LoggerTmplSignature
	template <typename... Args>
	inline void LoggerTmpl::LogFatalNLVariadic(int exitCode, ViewTy data, Args&&... args) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
		{
			LogFlagNLVariadic(data, L"FATAL", std::forward<Args>(args)...);
			LogFlagNLAppend(
				L"Logger [LogFatalNLVariadic] | Logger is terminating the program with the exit code : ",
				exitCode,
				L"FATAL"
			);
		}
		else
		{
			LogFlagNLVariadic(data, "FATAL", std::forward<Args>(args)...);
			LogFlagNLAppend(
				"Logger [LogFatalNLVariadic] | Logger is terminating the program with the exit code : ",
				exitCode,
				"FATAL"
			);
		}

		CM_BREAK_DEBUGGER();
		exit(exitCode);
	}

	LoggerTmplSignature
	template <typename... Args>
	inline void LoggerTmpl::LogFatalNLVariadicIf(bool condition, ViewTy data, Args&&... args) noexcept
	{
		if (condition)
			LogFatalNLVariadic(-1, data, std::forward<Args>(args)...);
	}

	LoggerTmplSignature
	template <typename... Args>
	inline void LoggerTmpl::LogFatalNLFormatted(ViewTy tag, ViewTy fmt, Args&&... args) noexcept
	{
		DataTy formatted = std::vformat(fmt, std::make_wformat_args(std::forward<Args>(args)...));

		LogFatalNLTagged(tag, formatted);
	}

	LoggerTmplSignature
	template <typename... Args>
	inline void LoggerTmpl::LogFatalNLFormattedIf(
		bool condition,
		ViewTy originTag,
		ViewTy fmt,
		Args&&... args
	) noexcept
	{
		if (condition)
			LogFatalNLFormatted(originTag, fmt, std::forward<Args>(args)...);
	}
#pragma endregion

	LoggerTmplSignature
	inline void LoggerTmpl::LogInline(ViewTy data) noexcept
	{
		if (!m_LogStream.is_open())
			return;

		CM_IF_DEBUG(
			if constexpr (S_IS_WIDE_LOGGER)
				std::wcout << data;
			else
				std::cout << data;
		);

		m_LogStream << data;
	}

	LoggerTmplSignature
	inline bool LoggerTmpl::OpenFile(ViewTy fileName) noexcept
	{
		if (m_LogStream.is_open())
			return false;

		m_LogStream.open(m_TargetFileName);

		if (!m_LogStream.is_open())
			return false;

		m_TargetFileName = fileName.data();

		if constexpr (S_IS_WIDE_LOGGER)
			LogInfo(L"Logger [OpenFile] | Logger successfully opened the file.\n");
		else
			LogInfo("Logger [OpenFile] | Logger successfully opened the file.\n");

		return true;
	}

	LoggerTmplSignature
	inline bool LoggerTmpl::OpenFileInDirectory(ViewTy fileName, const std::filesystem::path& directory) noexcept
	{
		if (m_LogStream.is_open())
			return false;

		std::filesystem::create_directories(directory);

		m_LogStream.open(fileName.data());

		if (!m_LogStream.is_open())
			return false;

		m_TargetFileName = fileName.data();

		if constexpr (S_IS_WIDE_LOGGER)
			LogInfo(L"Logger [OpenFileInDirectory] | Logger successfully opened the file.\n");
		else
			LogInfo("Logger [OpenFileInDirectory] | Logger successfully opened the file.\n");

		return true;
	}

	LoggerTmplSignature
	inline void LoggerTmpl::CloseFile() noexcept
	{
		if (!m_LogStream.is_open())
			return;

		if constexpr (S_IS_WIDE_LOGGER)
			LogInfo(L"Logger [CloseFile] | Stream is being closed.\n");
		else
			LogInfo("Logger [CloseFile] | Stream is being closed.\n");

		m_LogStream.flush();
		m_LogStream.close();
		m_LogStream.clear();
	}
#pragma endregion

#pragma region Private
#pragma region LogFlag
	LoggerTmplSignature
	inline void LoggerTmpl::LogFlag(ViewTy data, ViewTy flag) noexcept
	{
		if (!m_LogStream.is_open())
			return;

		if constexpr (S_IS_WIDE_LOGGER)
		{
			CM_IF_DEBUG(std::wcout << S_LOG_TAG << L'(' << flag << L") " << data);

			m_LogStream << S_LOG_TAG << L'(' << flag << L") " << data;
		}
		else
		{
			CM_IF_DEBUG(std::cout << S_LOG_TAG << '(' << flag << ") " << data);

			m_LogStream << S_LOG_TAG << '(' << flag << ") " << data;
		}
	}

	LoggerTmplSignature
	inline void LoggerTmpl::LogFlagTagged(ViewTy data, ViewTy flag, ViewTy tag) noexcept
	{
		if (!m_LogStream.is_open())
			return;

		if constexpr (S_IS_WIDE_LOGGER)
		{
			CM_IF_DEBUG(std::wcout << S_LOG_TAG << L'(' << flag << L") " << tag << data);

			m_LogStream << S_LOG_TAG << L'(' << flag << L") " << tag << data;
		}
		else
		{
			CM_IF_DEBUG(std::cout << S_LOG_TAG << '(' << flag << ") " << tag << data);

			m_LogStream << S_LOG_TAG << '(' << flag << ") " << tag << data;
		}
	}

	LoggerTmplSignature
	inline void LoggerTmpl::LogFlagNL(ViewTy data, ViewTy flag) noexcept
	{
		if (!m_LogStream.is_open())
			return;

		LogFlag(data, flag);

		if constexpr (S_IS_WIDE_LOGGER)
		{
			CM_IF_DEBUG(std::wcout << L'\n');
			m_LogStream << L'\n';
		}
		else
		{
			CM_IF_DEBUG(std::cout << '\n');
			m_LogStream << '\n';
		}
	}

	LoggerTmplSignature
	inline void LoggerTmpl::LogFlagTaggedNL(ViewTy data, ViewTy flag, ViewTy tag) noexcept
	{
		if (!m_LogStream.is_open())
			return;

		LogFlagTagged(data, flag, tag);

		if constexpr (S_IS_WIDE_LOGGER)
		{
			CM_IF_DEBUG(std::wcout << L'\n');
			m_LogStream << L'\n';
		}
		else
		{
			CM_IF_DEBUG(std::cout << '\n');
			m_LogStream << '\n';
		}
	}

	LoggerTmplSignature
	template <typename AppendTy>
	inline void LoggerTmpl::LogFlagNLAppend(ViewTy data, AppendTy append, ViewTy flag) noexcept
	{
		if (!m_LogStream.is_open())
			return;

		if constexpr (S_IS_WIDE_LOGGER)
		{
			CM_IF_DEBUG(std::wcout << S_LOG_TAG << L'(' << flag << L") " << data << append << L'\n');

			m_LogStream << S_LOG_TAG << L'(' << flag << L") " << data << append << L'\n';
		}
		else
		{
			CM_IF_DEBUG(std::cout << S_LOG_TAG << '(' << flag << ") " << data << append << L'\n');

			m_LogStream << S_LOG_TAG << '(' << flag << ") " << data << append << '\n';
		}
	}

	LoggerTmplSignature
	template <typename... Args>
	inline void LoggerTmpl::LogFlagNLVariadic(ViewTy data, ViewTy flag, Args&&... args) noexcept
	{
		if (!m_LogStream.is_open())
			return;

		if constexpr (S_IS_WIDE_LOGGER)
		{
			CM_IF_DEBUG(
				std::wcout << S_LOG_TAG << L'(' << flag << L") " << data;
			(std::wcout << ... << args);
			std::wcout << L'\n';
				);

			m_LogStream << S_LOG_TAG << L'(' << flag << L") " << data;
			(m_LogStream << ... << args);
			m_LogStream << L'\n';
		}
		else
		{
			CM_IF_DEBUG(
				std::cout << S_LOG_TAG << '(' << flag << ") " << data;
			(std::cout << ... << args);
			std::cout << '\n';
				);

			m_LogStream << S_LOG_TAG << L'(' << flag << L") " << data;
			(m_LogStream << ... << args);
			m_LogStream << L'\n';
		}
	}
#pragma endregion
#pragma endregion

	#undef LoggerTmplSignature
	#undef LoggerTmpl

	using LoggerWide = Logger<LoggerType::CM_WIDE_LOGGER, std::wstring, std::wstring_view, std::wofstream>;
	using LoggerNarrow = Logger<LoggerType::CM_NARROW_LOGGER, std::string, std::string_view, std::ofstream>;
}