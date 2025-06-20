#pragma once

#include <iostream>
#include <fstream>
#include <string_view>
#include <type_traits>
#include <filesystem>
#include <format>

#include "CMC_Macros.hpp"

/*
 *
 *	CMLogger :
 *
 *		A fail - fast - type logger where fatal logs == program termination.
 *      I spent too much time writing this to not be biased, but this is clearly
 *		the best logger ever written.
 *
 *	-- mb-07fw.
 *
 */

namespace CMCommon
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

	enum class CMLoggerState
	{
		INVALID = -1, 
		TARGETED,
		UNTARGETED
	};

	enum class CMLoggerType
	{
		CM_WIDE_LOGGER,
		CM_NARROW_LOGGER
	};

	template <CMLoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy>
		requires LoggerDataType<DataTy> && LoggerViewType<ViewTy> && ValidStream<StreamTy>
	class CMLogger
	{
	public:
		inline CMLogger(ViewTy targetFileName) noexcept;
		inline CMLogger() = default;
		inline ~CMLogger() noexcept;
	public:
		inline void LogInfo(ViewTy data) noexcept;
		inline void LogInfoNL(ViewTy data) noexcept;

		template <typename AppendTy>
		inline void LogInfoNLAppend(ViewTy data, AppendTy append) noexcept;

		template <typename... Args>
		inline void LogInfoNLVariadic(ViewTy data, Args&&... args) noexcept;

		inline void LogWarning(ViewTy data) noexcept;
		inline void LogWarningNL(ViewTy data) noexcept;

		template <typename AppendTy>
		inline void LogWarningNLAppend(ViewTy data, AppendTy append) noexcept;

		template <typename... Args>
		inline void LogWarningNLVariadic(ViewTy data, Args&&... args) noexcept;

		inline void LogFatal(ViewTy data, int exitCode = -1) noexcept;
		inline void LogFatalTagged(ViewTy tag, ViewTy data, int exitCode = -1) noexcept;
		inline void LogFatalNL(ViewTy data, int exitCode = -1) noexcept;
		inline void LogFatalNLTagged(ViewTy tag, ViewTy data, int exitCode = -1) noexcept;

		template <typename... Args>
		inline void LogFatalNLFormatted(ViewTy tag, ViewTy fmt, Args&&... args) noexcept;

		template <typename AppendTy>
		inline void LogFatalNLAppend(ViewTy data, AppendTy append, int exitCode = -1) noexcept;

		template <typename... Args>
		inline void LogFatalNLVariadic(int exitCode, ViewTy data, Args&&... args) noexcept;

		inline void LogInline(ViewTy data) noexcept;

		inline bool LogWarningNLIf(bool condition, ViewTy data) noexcept;
		inline void LogFatalNLIf(bool condition, ViewTy data, int exitCode = -1) noexcept;

		template <typename AppendTy>
		inline bool LogWarningNLAppendIf(bool condition, ViewTy data, AppendTy appendData) noexcept;

		template <typename AppendTy>
		inline void LogFatalNLAppendIf(bool condition, ViewTy data, AppendTy appendData, int exitCode = -1) noexcept;

		template <typename... Args>
		inline bool LogWarningNLVariadicIf(bool condition, ViewTy data, Args&&... args) noexcept;

		template <typename... Args>
		inline void LogFatalNLVariadicIf(bool condition, ViewTy data, Args&&... args) noexcept;

		template <typename... Args>
		inline void LogFatalNLFormattedIf(bool condition, ViewTy originTag, ViewTy fmt, Args&&... args) noexcept;

		inline void OpenFile(ViewTy fileName) noexcept;
		inline void OpenFileInDirectory(ViewTy fileName, const std::filesystem::path& directory) noexcept;
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
				return ViewTy(L"CMLoggerWide ");
			else
				return ViewTy("CMLogger ");
		}();
	private:
		DataTy m_TargetFileName;
		StreamTy m_LogStream;
	};

#pragma region CMLogger
#pragma region Public
	template <CMLoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy>
		requires LoggerDataType<DataTy>&& LoggerViewType<ViewTy>&& ValidStream<StreamTy>
	inline CMLogger<EnumLoggerType, DataTy, ViewTy, StreamTy>::CMLogger(ViewTy targetFileName) noexcept
		: m_LogStream(targetFileName.data()),
		  m_TargetFileName(targetFileName.data())
	{
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy>
		requires LoggerDataType<DataTy>&& LoggerViewType<ViewTy>&& ValidStream<StreamTy>
	inline CMLogger<EnumLoggerType, DataTy, ViewTy, StreamTy>::~CMLogger() noexcept
	{
		CloseFile();
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy>
		requires LoggerDataType<DataTy>&& LoggerViewType<ViewTy>&& ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, ViewTy, StreamTy>::LogInfo(ViewTy data) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
			LogFlag(data, L"INFO");
		else
			LogFlag(data, "INFO");
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy>
		requires LoggerDataType<DataTy>&& LoggerViewType<ViewTy>&& ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, ViewTy, StreamTy>::LogWarning(ViewTy data) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
			LogFlag(data, L"WARNING");
		else
			LogFlag(data, "WARNING");
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy>
		requires LoggerDataType<DataTy>&& LoggerViewType<ViewTy>&& ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, ViewTy, StreamTy>::LogFatal(ViewTy data, int exitCode) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
		{
			LogFlag(data, L"FATAL");
			LogFlagNLAppend(
				L"CMLogger [LogFatal] | Logger is terminating the program with the exit code : ",
				exitCode,
				L"FATAL"
			);
		}
		else
		{
			LogFlag(data, "FATAL");

			LogFlagNLAppend(
				"CMLogger [LogFatal] | Logger is terminating the program with the exit code : ",
				exitCode,
				"FATAL"
			);
		}

		CM_BREAK_DEBUGGER();
		exit(exitCode);
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy>
		requires LoggerDataType<DataTy>&& LoggerViewType<ViewTy>&& ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, ViewTy, StreamTy>::LogFatalTagged(ViewTy tag, ViewTy data, int exitCode) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
		{
			LogFlagTagged(data, L"FATAL", tag);
			LogFlagNLAppend(
				L"CMLogger [LogFatal] | Logger is terminating the program with the exit code : ",
				exitCode,
				L"FATAL"
			);
		}
		else
		{
			LogFlagTagged(data, "FATAL", tag);

			LogFlagNLAppend(
				"CMLogger [LogFatal] | Logger is terminating the program with the exit code : ",
				exitCode,
				"FATAL"
			);
		}

		CM_BREAK_DEBUGGER();
		exit(exitCode);
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy>
		requires LoggerDataType<DataTy>&& LoggerViewType<ViewTy>&& ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, ViewTy, StreamTy>::LogInline(ViewTy data) noexcept
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

#pragma region Conditionals
	template <CMLoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy>
		requires LoggerDataType<DataTy>&& LoggerViewType<ViewTy>&& ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, ViewTy, StreamTy>::LogFatalNLIf(bool condition, ViewTy data, int exitCode) noexcept
	{
		if (condition)
			LogFatalNL(data, exitCode);
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy>
		requires LoggerDataType<DataTy>&& LoggerViewType<ViewTy>&& ValidStream<StreamTy>
	inline bool CMLogger<EnumLoggerType, DataTy, ViewTy, StreamTy>::LogWarningNLIf(bool condition, ViewTy data) noexcept
	{
		if (!condition)
			return false;

		LogWarningNL(data);
		return true;
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy>
		requires LoggerDataType<DataTy>&& LoggerViewType<ViewTy>&& ValidStream<StreamTy>
	template <typename AppendTy>
	inline bool CMLogger<EnumLoggerType, DataTy, ViewTy, StreamTy>::LogWarningNLAppendIf(bool condition, ViewTy data, AppendTy append) noexcept
	{
		if (!condition)
			return false;

		LogWarningNLAppend(data, append);
		return true;
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy>
		requires LoggerDataType<DataTy>&& LoggerViewType<ViewTy>&& ValidStream<StreamTy>
	template <typename AppendTy>
	inline void CMLogger<EnumLoggerType, DataTy, ViewTy, StreamTy>::LogFatalNLAppendIf(bool condition, ViewTy data, AppendTy append, int exitCode) noexcept
	{
		if (condition)
			LogFatalNLAppend(data, append, exitCode);
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy>
		requires LoggerDataType<DataTy>&& LoggerViewType<ViewTy>&& ValidStream<StreamTy>
	template <typename... Args>
	inline bool CMLogger<EnumLoggerType, DataTy, ViewTy, StreamTy>::LogWarningNLVariadicIf(bool condition, ViewTy data, Args&&... args) noexcept
	{
		if (!condition)
			return false;

		LogWarningNLVariadic(data, std::forward<Args>(args)...);
		return true;
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy>
		requires LoggerDataType<DataTy>&& LoggerViewType<ViewTy>&& ValidStream<StreamTy>
	template <typename... Args>
	inline void CMLogger<EnumLoggerType, DataTy, ViewTy, StreamTy>::LogFatalNLVariadicIf(bool condition, ViewTy data, Args&&... args) noexcept
	{
		if (condition)
			LogFatalNLVariadic(-1, data, std::forward<Args>(args)...);
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy>
		requires LoggerDataType<DataTy>&& LoggerViewType<ViewTy>&& ValidStream<StreamTy>
	template <typename... Args>
	inline void CMLogger<EnumLoggerType, DataTy, ViewTy, StreamTy>::LogFatalNLFormattedIf(
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

#pragma region NL
	template <CMLoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy>
		requires LoggerDataType<DataTy>&& LoggerViewType<ViewTy>&& ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, ViewTy, StreamTy>::LogInfoNL(ViewTy data) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
			LogFlagNL(data, L"INFO");
		else
			LogFlagNL(data, "INFO");
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy>
		requires LoggerDataType<DataTy>&& LoggerViewType<ViewTy>&& ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, ViewTy, StreamTy>::LogWarningNL(ViewTy data) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
			LogFlagNL(data, L"WARNING");
		else
			LogFlagNL(data, "WARNING");
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy>
		requires LoggerDataType<DataTy>&& LoggerViewType<ViewTy>&& ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, ViewTy, StreamTy>::LogFatalNL(ViewTy data, int exitCode) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
		{
			LogFlagNL(data, L"FATAL");

			LogFlagNLAppend(
				L"CMLogger [LogFatalNL] | Logger is terminating the program with the exit code : ",
				exitCode,
				L"FATAL"
			);
		}
		else
		{
			LogFlagNL(data, "FATAL");

			LogFlagNLAppend(
				"CMLogger [LogFatalNL] | Logger is terminating the program with the exit code : ",
				exitCode,
				"FATAL"
			);
		}

		CM_BREAK_DEBUGGER();
		exit(exitCode);
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy>
		requires LoggerDataType<DataTy>&& LoggerViewType<ViewTy>&& ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, ViewTy, StreamTy>::LogFatalNLTagged(ViewTy tag, ViewTy data, int exitCode) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
		{
			LogFlagTaggedNL(data, L"FATAL", tag);

			LogFlagNLAppend(
				L"CMLogger [LogFatalNL] | Logger is terminating the program with the exit code : ",
				exitCode,
				L"FATAL"
			);
		}
		else
		{
			LogFlagTaggedNL(data, "FATAL", tag);

			LogFlagNLAppend(
				"CMLogger [LogFatalNL] | Logger is terminating the program with the exit code : ",
				exitCode,
				"FATAL"
			);
		}

		CM_BREAK_DEBUGGER();
		exit(exitCode);
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy>
		requires LoggerDataType<DataTy>&& LoggerViewType<ViewTy>&& ValidStream<StreamTy>
	template <typename... Args>
	inline void CMLogger<EnumLoggerType, DataTy, ViewTy, StreamTy>::LogFatalNLFormatted(ViewTy tag, ViewTy fmt, Args&&... args) noexcept
	{
		DataTy formatted = std::vformat(fmt, std::make_wformat_args(std::forward<Args>(args)...));

		LogFatalNLTagged(tag, formatted);
	}
#pragma endregion

#pragma region NLAppend
	template <CMLoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy>
		requires LoggerDataType<DataTy>&& LoggerViewType<ViewTy>&& ValidStream<StreamTy>
	template <typename AppendTy>
	inline void CMLogger<EnumLoggerType, DataTy, ViewTy, StreamTy>::LogInfoNLAppend(ViewTy data, AppendTy append) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
			LogFlagNLAppend(data, append, L"INFO");
		else
			LogFlagNLAppend(data, append, "INFO");
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy>
		requires LoggerDataType<DataTy>&& LoggerViewType<ViewTy>&& ValidStream<StreamTy>
	template <typename AppendTy>
	inline void CMLogger<EnumLoggerType, DataTy, ViewTy, StreamTy>::LogWarningNLAppend(ViewTy data, AppendTy append) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
			LogFlagNLAppend(data, append, L"WARNING");
		else
			LogFlagNLAppend(data, append, "WARNING");
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy>
		requires LoggerDataType<DataTy>&& LoggerViewType<ViewTy>&& ValidStream<StreamTy>
	template <typename AppendTy>
	inline void CMLogger<EnumLoggerType, DataTy, ViewTy, StreamTy>::LogFatalNLAppend(ViewTy data, AppendTy append, int exitCode) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
		{
			LogFlagNLAppend(data, append, L"FATAL");

			LogFlagNLAppend(
				L"CMLogger [LogFatalNLAppend] | Logger is terminating the program with the exit code : ",
				exitCode,
				L"FATAL"
			);
		}
		else
		{
			LogFlagNLAppend(data, append, "FATAL");

			LogFlagNLAppend(
				"CMLogger [LogFatalNLAppend] | Logger is terminating the program with the exit code : ",
				exitCode,
				"FATAL"
			);
		}

		CM_BREAK_DEBUGGER();
		exit(exitCode);
	}
#pragma endregion

#pragma region NLVariadic
	template <CMLoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy>
		requires LoggerDataType<DataTy>&& LoggerViewType<ViewTy>&& ValidStream<StreamTy>
	template <typename... Args>
	inline void CMLogger<EnumLoggerType, DataTy, ViewTy, StreamTy>::LogInfoNLVariadic(ViewTy data, Args&&... args) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
			LogFlagNLVariadic(data, L"INFO", std::forward<Args>(args)...);
		else
			LogFlagNLVariadic(data, "INFO", std::forward<Args>(args)...);
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy>
		requires LoggerDataType<DataTy>&& LoggerViewType<ViewTy>&& ValidStream<StreamTy>
	template <typename... Args>
	inline void CMLogger<EnumLoggerType, DataTy, ViewTy, StreamTy>::LogWarningNLVariadic(ViewTy data, Args&&... args) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
			LogFlagNLVariadic(data, L"WARNING", std::forward<Args>(args)...);
		else
			LogFlagNLVariadic(data, "WARNING", std::forward<Args>(args)...);
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy>
		requires LoggerDataType<DataTy>&& LoggerViewType<ViewTy>&& ValidStream<StreamTy>
	template <typename... Args>
	inline void CMLogger<EnumLoggerType, DataTy, ViewTy, StreamTy>::LogFatalNLVariadic(int exitCode, ViewTy data, Args&&... args) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
		{
			LogFlagNLVariadic(data, L"FATAL", std::forward<Args>(args)...);
			LogFlagNLAppend(
				L"CMLogger [LogFatalNLVariadic] | Logger is terminating the program with the exit code : ",
				exitCode,
				L"FATAL"
			);
		}
		else
		{
			LogFlagNLVariadic(data, "FATAL", std::forward<Args>(args)...);
			LogFlagNLAppend(
				"CMLogger [LogFatalNLVariadic] | Logger is terminating the program with the exit code : ",
				exitCode,
				"FATAL"
			);
		}

		CM_BREAK_DEBUGGER();
		exit(exitCode);
	}
#pragma endregion

#pragma region Stream Management
	template <CMLoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy>
		requires LoggerDataType<DataTy>&& LoggerViewType<ViewTy>&& ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, ViewTy, StreamTy>::OpenFile(ViewTy fileName) noexcept
	{
		if (m_LogStream.is_open())
			return;

		m_LogStream.open(m_TargetFileName);

		if (!m_LogStream.is_open())
			return;

		m_TargetFileName = fileName.data();

		if constexpr (S_IS_WIDE_LOGGER)
			LogInfo(L"CMLogger [OpenFile] | Logger successfully opened the file.\n");
		else
			LogInfo("CMLogger [OpenFile] | Logger successfully opened the file.\n");
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy>
		requires LoggerDataType<DataTy>&& LoggerViewType<ViewTy>&& ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, ViewTy, StreamTy>::OpenFileInDirectory(ViewTy fileName, const std::filesystem::path& directory) noexcept
	{
		if (m_LogStream.is_open())
			return;

		bool directoryExisted = std::filesystem::create_directories(directory);

		m_LogStream.open(fileName.data());

		if (!m_LogStream.is_open())
			return;

		m_TargetFileName = fileName.data();

		if constexpr (S_IS_WIDE_LOGGER)
			LogInfo(L"CMLogger [OpenFileInDirectory] | Logger successfully opened the file.\n");
		else
			LogInfo("CMLogger [OpenFileInDirectory] | Logger successfully opened the file.\n");
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy>
		requires LoggerDataType<DataTy>&& LoggerViewType<ViewTy>&& ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, ViewTy, StreamTy>::CloseFile() noexcept
	{
		if (!m_LogStream.is_open())
			return;

		if constexpr (S_IS_WIDE_LOGGER)
			LogInfo(L"CMLogger [CloseFile] | Stream is being closed.\n");
		else
			LogInfo("CMLogger [CloseFile] | Stream is being closed.\n");

		m_LogStream.close();
	}
#pragma endregion

#pragma endregion

#pragma region Private
#pragma region LogFlag
	template <CMLoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy>
		requires LoggerDataType<DataTy>&& LoggerViewType<ViewTy>&& ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, ViewTy, StreamTy>::LogFlag(ViewTy data, ViewTy flag) noexcept
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

	template <CMLoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy>
		requires LoggerDataType<DataTy>&& LoggerViewType<ViewTy>&& ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, ViewTy, StreamTy>::LogFlagTagged(ViewTy data, ViewTy flag, ViewTy tag) noexcept
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

	template <CMLoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy>
		requires LoggerDataType<DataTy>&& LoggerViewType<ViewTy>&& ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, ViewTy, StreamTy>::LogFlagNL(ViewTy data, ViewTy flag) noexcept
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

	template <CMLoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy>
		requires LoggerDataType<DataTy>&& LoggerViewType<ViewTy>&& ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, ViewTy, StreamTy>::LogFlagTaggedNL(ViewTy data, ViewTy flag, ViewTy tag) noexcept
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

	template <CMLoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy>
		requires LoggerDataType<DataTy>&& LoggerViewType<ViewTy>&& ValidStream<StreamTy>
	template <typename AppendTy>
	inline void CMLogger<EnumLoggerType, DataTy, ViewTy, StreamTy>::LogFlagNLAppend(ViewTy data, AppendTy append, ViewTy flag) noexcept
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

	template <CMLoggerType EnumLoggerType, typename DataTy, typename ViewTy, typename StreamTy>
		requires LoggerDataType<DataTy>&& LoggerViewType<ViewTy>&& ValidStream<StreamTy>
	template <typename... Args>
	inline void CMLogger<EnumLoggerType, DataTy, ViewTy, StreamTy>::LogFlagNLVariadic(ViewTy data, ViewTy flag, Args&&... args) noexcept
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

	using CMLoggerWide = CMLogger<CMLoggerType::CM_WIDE_LOGGER, std::wstring, std::wstring_view, std::wofstream>;
	using CMLoggerNarrow = CMLogger<CMLoggerType::CM_NARROW_LOGGER, std::string, std::string_view, std::ofstream>;
}