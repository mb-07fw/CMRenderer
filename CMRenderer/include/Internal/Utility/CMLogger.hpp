#pragma once

#include "Core/CMMacros.hpp"

#include <iostream>
#include <string_view>
#include <fstream>
#include <type_traits>

namespace CMRenderer::Utility
{
	template <typename Ty>
	struct IsLoggableType
		: std::bool_constant<
			std::is_same_v<Ty, std::string_view> ||
			std::is_same_v<Ty, std::wstring_view>> {
	};

	template <typename Ty>
	concept LoggableType = IsLoggableType<Ty>::value;



	template <typename Ty>
	struct IsValidStream
		: std::bool_constant<
			std::is_same_v<Ty, std::ofstream> ||
			std::is_same_v<Ty, std::wofstream>> {
	};

	template <typename Ty>
	concept ValidStream = IsValidStream<Ty>::value;



	enum class CMLoggerState
	{
		INVALID = -1, TARGETED, UNTARGETED
	};

	enum class CMLoggerType
	{
		CM_WIDE_LOGGER,
		CM_NARROW_LOGGER
	};



	template <CMLoggerType EnumLoggerType, typename DataTy, typename StreamTy>
		requires LoggableType<DataTy> && ValidStream<StreamTy>
	class CMLogger
	{
	public:
		inline CMLogger(DataTy targetFileName) noexcept;
		inline CMLogger() noexcept;
		inline ~CMLogger() noexcept;
	public:
		inline void LogInfo(DataTy data) noexcept;
		inline void LogInfoNL(DataTy data) noexcept;

		template <typename AppendTy>
		inline void LogInfoNLAppend(DataTy data, AppendTy append) noexcept;

		template <typename... Args>
		inline void LogInfoNLVariadic(DataTy data, Args&&... args) noexcept;

		inline void LogWarning(DataTy data) noexcept;
		inline void LogWarningNL(DataTy data) noexcept;

		template <typename AppendTy>
		inline void LogWarningNLAppend(DataTy data, AppendTy append) noexcept;

		template <typename... Args>
		inline void LogWarningNLVariadic(DataTy data, Args&&... args) noexcept;

		inline void LogFatal(DataTy data, int exitCode = -1) noexcept;
		inline void LogFatalNL(DataTy data, int exitCode = -1) noexcept;

		template <typename AppendTy>
		inline void LogFatalNLAppend(DataTy data, AppendTy append, int exitCode = -1) noexcept;

		template <typename... Args>
		inline void LogFatalNLVariadic(int exitCode, DataTy data, Args&&... args) noexcept;

		inline void LogInline(DataTy data) noexcept;

		inline void OpenStream() noexcept;
		inline void CloseStream() noexcept;

		inline void SetTargetFile(DataTy targetFileName) noexcept;

		inline [[nodiscard]] CMLoggerState CurrentState() const noexcept { return m_CurrentState; }
		inline [[nodiscard]] bool IsStreamOpen() const noexcept { return m_LogStream.is_open(); }
		inline [[nodiscard]] bool IsTargetFileSet() const noexcept { return m_TargetFileSet; }
	private:
		inline void LogFlag(DataTy data, const DataTy pFlag) noexcept;
		inline void LogFlagNL(DataTy data, const DataTy pFlag) noexcept;

		template <typename AppendTy>
		inline void LogFlagNLAppend(DataTy data, AppendTy append, const DataTy pFlag) noexcept;

		template <typename... Args>
		inline void LogFlagNLVariadic(DataTy data, const DataTy pFlag, Args&&... args) noexcept;

		inline void SetTag() noexcept;
	private:
		static constexpr bool S_IS_WIDE_LOGGER = std::is_same_v<DataTy, std::wstring_view>;
		DataTy m_LogTag;
		DataTy m_TargetFileName;
		StreamTy m_LogStream;
		CMLoggerState m_CurrentState = CMLoggerState::INVALID;
		bool m_TargetFileSet = false;
	};

#pragma region CMLogger
#pragma region Public
	template <CMLoggerType EnumLoggerType, typename DataTy, typename StreamTy>
		requires LoggableType<DataTy> && ValidStream<StreamTy>
	inline CMLogger<EnumLoggerType, DataTy, StreamTy>::CMLogger(DataTy targetFileName) noexcept	
	{
		SetTag();
		SetTargetFile(targetFileName);
		OpenStream();
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename StreamTy>
		requires LoggableType<DataTy>&& ValidStream<StreamTy>
	inline CMLogger<EnumLoggerType, DataTy, StreamTy>::CMLogger() noexcept
	{
		SetTag();
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename StreamTy>
		requires LoggableType<DataTy> && ValidStream<StreamTy>
	inline CMLogger<EnumLoggerType, DataTy, StreamTy>::~CMLogger() noexcept
	{
		CloseStream();
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename StreamTy>
		requires LoggableType<DataTy>&& ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, StreamTy>::LogInfo(DataTy data) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
			LogFlag(data, L"INFO");
		else
			LogFlag(data, "INFO");
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename StreamTy>
		requires LoggableType<DataTy>&& ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, StreamTy>::LogWarning(DataTy data) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
			LogFlag(data, L"WARNING");
		else
			LogFlag(data, "WARNING");
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename StreamTy>
		requires LoggableType<DataTy>&& ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, StreamTy>::LogFatal(DataTy data, int exitCode) noexcept
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

	template <CMLoggerType EnumLoggerType, typename DataTy, typename StreamTy>
		requires LoggableType<DataTy>&& ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, StreamTy>::LogInline(DataTy data) noexcept
	{
		if (!m_LogStream.is_open() || !m_TargetFileSet)
			return;

		CM_IF_DEBUG(
			if constexpr (S_IS_WIDE_LOGGER)
				std::wcout << data;
			else
				std::cout << data;
		);

		m_LogStream << data;
	}

#pragma region NL
	template <CMLoggerType EnumLoggerType, typename DataTy, typename StreamTy>
		requires LoggableType<DataTy>&& ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, StreamTy>::LogInfoNL(DataTy data) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
			LogFlagNL(data, L"INFO");
		else
			LogFlagNL(data, "INFO");
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename StreamTy>
		requires LoggableType<DataTy>&& ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, StreamTy>::LogWarningNL(DataTy data) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
			LogFlagNL(data, L"WARNING");
		else
			LogFlagNL(data, "WARNING");
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename StreamTy>
		requires LoggableType<DataTy>&& ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, StreamTy>::LogFatalNL(DataTy data, int exitCode) noexcept
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
#pragma endregion

#pragma region NLAppend
	template <CMLoggerType EnumLoggerType, typename DataTy, typename StreamTy>
		requires LoggableType<DataTy>&& ValidStream<StreamTy>
	template <typename AppendTy>
	inline void CMLogger<EnumLoggerType, DataTy, StreamTy>::LogInfoNLAppend(DataTy data, AppendTy append) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
			LogFlagNLAppend(data, append, L"INFO");
		else
			LogFlagNLAppend(data, append, "INFO");
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename StreamTy>
		requires LoggableType<DataTy>&& ValidStream<StreamTy>
	template <typename AppendTy>
	inline void CMLogger<EnumLoggerType, DataTy, StreamTy>::LogWarningNLAppend(DataTy data, AppendTy append) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
			LogFlagNLAppend(data, append, L"WARNING");
		else
			LogFlagNLAppend(data, append, "WARNING");
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename StreamTy>
		requires LoggableType<DataTy>&& ValidStream<StreamTy>
	template <typename AppendTy>
	inline void CMLogger<EnumLoggerType, DataTy, StreamTy>::LogFatalNLAppend(DataTy data, AppendTy append, int exitCode) noexcept
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
	template <CMLoggerType EnumLoggerType, typename DataTy, typename StreamTy>
		requires LoggableType<DataTy>&& ValidStream<StreamTy>
	template <typename... Args>
	inline void CMLogger<EnumLoggerType, DataTy, StreamTy>::LogInfoNLVariadic(DataTy data, Args&&... args) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
			LogFlagNLVariadic(data, L"INFO", std::forward<Args>(args)...);
		else
			LogFlagNLVariadic(data, "INFO", std::forward<Args>(args)...);
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename StreamTy>
		requires LoggableType<DataTy>&& ValidStream<StreamTy>
	template <typename... Args>
	inline void CMLogger<EnumLoggerType, DataTy, StreamTy>::LogWarningNLVariadic(DataTy data, Args&&... args) noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
			LogFlagNLVariadic(data, L"WARNING", std::forward<Args>(args)...);
		else
			LogFlagNLVariadic(data, "WARNING", std::forward<Args>(args)...);
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename StreamTy>
		requires LoggableType<DataTy>&& ValidStream<StreamTy>
	template <typename... Args>
	inline void CMLogger<EnumLoggerType, DataTy, StreamTy>::LogFatalNLVariadic(int exitCode, DataTy data, Args&&... args) noexcept
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
	template <CMLoggerType EnumLoggerType, typename DataTy, typename StreamTy>
		requires LoggableType<DataTy> && ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, StreamTy>::OpenStream() noexcept
	{
		if (m_LogStream.is_open() || !m_TargetFileSet)
			return;

		m_LogStream.open(m_TargetFileName.data());
		
		if (!m_LogStream.is_open())
		{
			m_CurrentState = CMLoggerState::INVALID;
			return;
		}

		m_CurrentState = CMLoggerState::TARGETED;

		if constexpr (S_IS_WIDE_LOGGER)
			LogInfo(L"CMLogger [OpenStream] | Logger successfully opened the file.\n");
		else
			LogInfo("CMLogger [OpenStream] | Logger successfully opened the file.\n");
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename StreamTy>
		requires LoggableType<DataTy> && ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, StreamTy>::CloseStream() noexcept
	{
		if (!m_LogStream.is_open())
			return;

		if constexpr (S_IS_WIDE_LOGGER)
			LogInfo(L"CMLogger [CloseStream] | Stream is being closed.\n");
		else
			LogInfo("CMLogger [CloseStream] | Stream is being closed.\n");

		m_LogStream.close();
		m_CurrentState = CMLoggerState::UNTARGETED;
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename StreamTy>
		requires LoggableType<DataTy>&& ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, StreamTy>::SetTargetFile(DataTy targetFileName) noexcept
	{
		m_TargetFileName = targetFileName;
		m_TargetFileSet = true;
	}
#pragma endregion
#pragma endregion

#pragma region Private
#pragma region LogFlag
	template <CMLoggerType EnumLoggerType, typename DataTy, typename StreamTy>
		requires LoggableType<DataTy> && ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, StreamTy>::LogFlag(DataTy data, const DataTy pFlag) noexcept
	{
		if (!m_LogStream.is_open() || !m_TargetFileSet)
			return;

		if constexpr (S_IS_WIDE_LOGGER)
		{
			CM_IF_DEBUG(std::wcout << m_LogTag << L'(' << pFlag << L") " << data);

			m_LogStream << m_LogTag << L'(' << pFlag << L") " << data;
		}
		else
		{
			CM_IF_DEBUG(std::cout << m_LogTag << '(' << pFlag << ") " << data);

			m_LogStream << m_LogTag << '(' << pFlag << ") " << data;
		}
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename StreamTy>
		requires LoggableType<DataTy>&& ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, StreamTy>::LogFlagNL(DataTy data, const DataTy pFlag) noexcept
	{
		LogFlag(data, pFlag);

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

	template <CMLoggerType EnumLoggerType, typename DataTy, typename StreamTy>
		requires LoggableType<DataTy>&& ValidStream<StreamTy>
	template <typename AppendTy>
	inline void CMLogger<EnumLoggerType, DataTy, StreamTy>::LogFlagNLAppend(DataTy data, AppendTy append, const DataTy pFlag) noexcept
	{
		if (!m_LogStream.is_open() || !m_TargetFileSet)
			return;

		if constexpr (S_IS_WIDE_LOGGER)
		{
			CM_IF_DEBUG(std::wcout << m_LogTag << L'(' << pFlag << L") " << data << append << L'\n');

			m_LogStream << m_LogTag << L'(' << pFlag << L") " << data << append << L'\n';
		}
		else
		{
			CM_IF_DEBUG(std::cout << m_LogTag << '(' << pFlag << ") " << data << append << L'\n');

			m_LogStream << m_LogTag << '(' << pFlag << ") " << data << append << '\n';
		}
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename StreamTy>
		requires LoggableType<DataTy>&& ValidStream<StreamTy>
	template <typename... Args>
	inline void CMLogger<EnumLoggerType, DataTy, StreamTy>::LogFlagNLVariadic(DataTy data, const DataTy pFlag, Args&&... args) noexcept
	{
		if (!m_LogStream.is_open() || !m_TargetFileSet)
			return;

		if constexpr (S_IS_WIDE_LOGGER)
		{
			CM_IF_DEBUG(
				std::wcout << m_LogTag << L'(' << pFlag << L") " << data;
				(std::wcout << ... << args);
				std::wcout << L'\n';
			);

			m_LogStream << m_LogTag << L'(' << pFlag << L") " << data;
			(m_LogStream << ... << args);
			m_LogStream << L'\n';
		}
		else
		{
			CM_IF_DEBUG(
				std::cout << m_LogTag << '(' << pFlag << ") " << data;
				(std::cout << ... << args);
				std::cout << '\n';
			);

			m_LogStream << m_LogTag << L'(' << pFlag << L") " << data;
			(m_LogStream << ... << args);
			m_LogStream << L'\n';
		}
	}
#pragma region

	// TODO : Turn this into a constexpr variable.
	template <CMLoggerType EnumLoggerType, typename DataTy, typename StreamTy>
		requires LoggableType<DataTy> && ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, StreamTy>::SetTag() noexcept
	{
		if constexpr (S_IS_WIDE_LOGGER)
			m_LogTag = L"[CMLoggerWide] ";
		else
			m_LogTag = "[CMLoggerNarrow] ";
	}
#pragma endregion
#pragma endregion

	using CMLoggerWide = CMLogger<CMLoggerType::CM_WIDE_LOGGER, std::wstring_view, std::wofstream>;
	using CMLoggerNarrow = CMLogger<CMLoggerType::CM_NARROW_LOGGER, std::string_view, std::ofstream>;
}