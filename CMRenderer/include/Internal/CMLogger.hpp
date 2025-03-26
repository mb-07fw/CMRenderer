#pragma once

#include <string_view>
#include <fstream>
#include <type_traits>

namespace CMRenderer
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
		inline void LogWarning(DataTy data) noexcept;
		inline void LogFatal(DataTy data, int exitCode = -1) noexcept;
		inline void LogInline(DataTy data) noexcept;

		inline void OpenStream() noexcept;
		inline void CloseStream() noexcept;

		inline void SetTargetFile(DataTy targetFileName) noexcept;

		inline [[nodiscard]] CMLoggerState CurrentState() const noexcept { return m_CurrentState; }
		inline [[nodiscard]] bool IsStreamOpen() const noexcept { return m_LogStream.is_open(); }
		inline [[nodiscard]] bool IsTargetFileSet() const noexcept { return m_TargetFileSet; }
	private:
		inline void LogFlag(DataTy data, const char* pFlag) noexcept;
		inline void SetTag() noexcept;
	private:
		static constexpr bool S_IS_WIDE_LOGGER = std::is_same_v<DataTy, std::wstring_view>;
		std::string m_LogTag;
		DataTy m_TargetFileName;
		StreamTy m_LogStream;
		CMLoggerState m_CurrentState = CMLoggerState::INVALID;
		bool m_TargetFileSet = false;
	};



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
		requires LoggableType<DataTy> && ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, StreamTy>::LogInfo(DataTy data) noexcept
	{
		LogFlag(data, "INFO");
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename StreamTy>
		requires LoggableType<DataTy> && ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, StreamTy>::LogWarning(DataTy data) noexcept
	{
		LogFlag(data, "WARNING");
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename StreamTy>
		requires LoggableType<DataTy> && ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, StreamTy>::LogFatal(DataTy data, int exitCode) noexcept
	{
		LogFlag(data, "FATAL");
		
		if constexpr (S_IS_WIDE_LOGGER)
		{
			std::wstring message = L"CMLogger [LogFatal] | Logger is terminating the program with the exit code : " + std::to_wstring(exitCode) + L'\n';
			LogFlag(message, "FATAL");
		}
		else
		{
			std::string message = "CMLogger [LogFatal] | Logger is terminating the program with the exit code : " + std::to_string(exitCode) + '\n';
			LogFlag(message, "FATAL");
		}

		//__debugbreak();
		exit(exitCode);
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename StreamTy>
		requires LoggableType<DataTy> && ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, StreamTy>::LogInline(DataTy data) noexcept
	{
		if (!m_LogStream.is_open() || !m_TargetFileSet)
			return;

#ifndef CM_DISTRIBUTION
		if constexpr (S_IS_WIDE_LOGGER)
			std::wcout << data.data();
		else
			std::cout << data.data();
#endif

		m_LogStream << data.data();
	}

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
			LogInfo("CMLogger [OpenStream] |Logger successfully opened the file.\n");
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

	template <CMLoggerType EnumLoggerType, typename DataTy, typename StreamTy>
		requires LoggableType<DataTy> && ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, StreamTy>::LogFlag(DataTy data, const char* pFlag) noexcept
	{
		if (!m_LogStream.is_open() || !m_TargetFileSet)
			return;

#ifndef CM_DISTRIBUTION
		if constexpr (S_IS_WIDE_LOGGER)
			std::wcout << m_LogTag.data() << '(' << pFlag << ") " << data.data();
		else
			std::cout << m_LogTag.data() << '(' << pFlag << ") " << data.data();
#endif

		m_LogStream << m_LogTag.data() << '(' << pFlag << ") " << data.data();
	}

	template <CMLoggerType EnumLoggerType, typename DataTy, typename StreamTy>
		requires LoggableType<DataTy> && ValidStream<StreamTy>
	inline void CMLogger<EnumLoggerType, DataTy, StreamTy>::SetTag() noexcept
	{
		if constexpr (EnumLoggerType == CMLoggerType::CM_WIDE_LOGGER)
			m_LogTag = "[CMLoggerWide] ";
		else if constexpr (EnumLoggerType == CMLoggerType::CM_NARROW_LOGGER)
			m_LogTag = "[CMLoggerNarrow] ";
		else
			m_LogTag = "[CMLogger] ";
	}

	using CMLoggerWide = CMLogger<CMLoggerType::CM_WIDE_LOGGER, std::wstring_view, std::wofstream>;
	using CMLoggerNarrow = CMLogger<CMLoggerType::CM_NARROW_LOGGER, std::string_view, std::ofstream>;
}