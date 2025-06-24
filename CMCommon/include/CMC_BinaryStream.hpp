#pragma once

#include <fstream>
#include <filesystem>
#include <type_traits>
#include <concepts>
#include <limits>
#include <array>
#include <vector>
#include <span>

#include "CMC_Utility.hpp"

namespace CMCommon
{
	enum class CMBinaryStreamType : uint8_t
	{
		INVALID,
		READ,
		WRITE,
		READ_WRITE
	};

	enum class CMBinaryStreamFileStateType : uint8_t
	{
		INVALID,	/* Invalid state. */
		ABSENT,		/* File path was provided but doesn't exist. */
		SET,		/* File path is valid and was previously set. */
		UNTARGETED, /* File path was valid but was untargeted due to stream being closed. */
		TARGETED	/* File path is valid and is currently targeted by the stream. */
	};

	enum class CMBinaryStreamOperationType : uint8_t
	{
		INVALID,
		SET,
		READ,
		WRITE,
		FLUSH,
		CLEAR,
		SEEK_BEGINNING_IN,
		SEEK_IN,
		SEEK_BEGINNING_OUT,
		OPEN,
		CLOSE
	};

	template <bool RestrictRead, bool RestrictWrite>
		requires (!(RestrictRead&& RestrictWrite))
	class CMBinaryStream;

	template <bool RestrictRead, typename T>
	concept HasSaveFunc = requires(const T & data, CMBinaryStream<RestrictRead, false>&binaryStream) {
		{ data.Save(binaryStream) } -> std::same_as<bool>;
	};

	template <bool RestrictWrite, typename T>
	concept HasLoadFunc = requires(T & data, CMBinaryStream<false, RestrictWrite>&binaryStream) {
		{ data.Load(binaryStream) } -> std::same_as<std::streamsize>;
	};

	/* NOTE : Only read or write access can be restricted at once. (Both cannot be true) */
	template <bool RestrictRead = false, bool RestrictWrite = false>
		requires (!(RestrictRead&& RestrictWrite))
	class CMBinaryStream
	{
	public:
		inline CMBinaryStream(CMBinaryStreamType streamType, const std::filesystem::path& filePath = std::filesystem::path()) noexcept;
		~CMBinaryStream() = default;
		CMBinaryStream() = default;
	public:
		/* Sets the CMBinaryStream's CMBinaryStreamType and current file.
		 * Asserts @streamType to coincide with read or write restrictions.
		 * (i.e., if WriteAccess is restricted, and streamType == WRITE or READ_WRITE, the stream
		 *   is left in an invalid state)
		 * Closes the std::fstream if its currently open.
		 * Stream must be opened after being set by calling Open().
		 * Returns true if set state is valid; false otherwise.
		 */
		inline [[nodiscard]] bool Set(CMBinaryStreamType streamType, const std::filesystem::path& filePath) noexcept;

		/* Opens the current file path with the current CMBinaryStreamType configuration.
		 * No-op if the stream is currently open, or a valid file path isn't present.
		 */
		inline void Open() noexcept;

		/* Closes the stream if its currently open.
		 * No-op if the stream is already closed.
		 */
		inline void Close() noexcept;

		/* Flushes the stream, which forces any buffered output data to be written
		 *   to disk, as per std::fstream::flush()'s specification.
		 * This should be called before reading input if the stream was written to previously.
		 * Returns true if the stream is in a valid state; false otherwise.
		 */
		inline bool Flush() noexcept
			requires (!RestrictWrite);

		/* Clears the stream of any error flags.
		 * Returns true if the stream was in a valid state before or after clearing;
		 *   false otherwise.
		 */
		inline bool Clear() noexcept;

		/* Sets the file cursor (get pointer) to the beginning of the file.
		 * Returns true if the stream is in a valid state; false otherwise.
		 */
		inline bool SeekBeginningIn() noexcept
			requires (!RestrictRead);

		/* Sets the file cursor (put pointer) to the beginning of the file.
		 * Returns true if the stream is in a valid state; false otherwise.
		 */
		inline bool SeekBeginningOut() noexcept
			requires (!RestrictWrite);

		/* Sets the file cursor (get pointer) to the provided byte position.
		 * Returns false if @bytePos is negative or exceeds the file size, or the stream is
		 *   in an invalid state; true otherwise.
		 */
		inline bool SeekIn(std::streamsize bytePos) noexcept
			requires (!RestrictRead);

		template <typename Ty>
			requires (!RestrictWrite && (std::is_trivially_copyable_v<Ty> || HasSaveFunc<RestrictRead, Ty>))
		/* Writes @data to the stream by writing it's memory based off of it's byte size.
		 * Returns true if IsOutput() and the stream is in a valid state before and after attempting output;
		 *   false if otherwise.
		 * If @Ty isn't a trivially copyable type, it is expected that the type handles
		 *   serialization natively via the member function @data.Save(CMBinaryStream<RestrictRead, false>& binaryStream).
		 * Requires RestrictWrite to be false (CMBinaryStreamOutput or CMBinaryStream<..., false>)
		 *   and type @Ty to be trivially copyable, or has defined the member function
		 *   @data.Save(const CMBinaryStream& binaryStream).
		 */
		inline bool Write(const Ty& data) noexcept;

		template <typename Ty>
			requires (!RestrictWrite && (std::is_trivially_copyable_v<Ty> || HasSaveFunc<RestrictRead, Ty>))
		/* Writes @data to the stream by writing it's memory based off of it's byte size (sizeof(@Ty) * @data.size()).
		 * Only the vector's content itself is written; the data is useless if the vector's size is unknown.
		 *   Because of this, you should serialize a vector's size alongside it.
		 * Returns true if IsOutput() and the stream is in a valid state before and after attempting output;
		 *   false if otherwise or @data.empty().
		 * Requires RestrictWrite to be false (CMBinaryStreamOutput or CMBinaryStream<..., false>)
		 *   and type @Ty to be trivially copyable, or has defined the member function
		 *   @data.Save(const CMBinaryStream& binaryStream).
		 */
		inline bool Write(const std::vector<Ty>& data) noexcept;

		/* Writes @str to the stream by writing it's memory based off of it's length (sizeof(char) * @str.length()).
		 * Only the string's content itself is written; the data is useless if the string's length is unknown.
		 *   Because of this, you should serialize a string's length alongside it.
		 * Returns true if  IsOutput() and the stream is in a valid state before and after attempting output; 
		 *   false if otherwise or @str.empty().
		 * Requires RestrictWrite to be false (CMBinaryStreamOutput or CMBinaryStream<..., false>)
		 *   and type @Ty to be trivially copyable.
		 */
		inline bool Write(const std::string& str) noexcept
			requires (!RestrictWrite);

		/* Writes @numBytes into the stream from @pData.
		 * Returns true if IsOutput() and the stream is in a valid state before and after attempting output;
		 *   false otherwise.
		 * Requires RestrictWrite to be false (CMBinaryStreamOutput or CMBinaryStream<..., false>).
		 */
		inline bool WriteRaw(const void* pData, std::streamsize numBytes) noexcept
			requires (!RestrictWrite);

		template <typename Ty>
			requires (!RestrictRead && (std::is_trivially_copyable_v<Ty> || HasLoadFunc<RestrictWrite, Ty>))
		/* Reads sizeof(@Ty) bytes from the stream and reinterprets them as type @Ty into @outData.
		 * Returns the number of bytes read.
		 * Returns -1 if !IsInput() or the stream is in an invalid state.
		 * If EOF is reached, returns the number of bytes read from the partial read.
		 * If EOF is reached, @outData will only be set if the number of bytes read fully
		 *   encompasses sizeof(@Ty).
		 * If @Ty isn't a trivially copyable type, it is expected that the type handles
		 *   de-serialization natively via the member function @outData.Load(CMBinaryStream<false, RestrictWrite>& binaryStream).
		 * Requires RestrictRead to be false (CMBinaryStreamInput or CMBinaryStream<false, ...>)
		 *   and type @Ty to be trivially copyable, or has defined the member function
		 *   @outData.Load(const CMBinaryStream<false, false>& binaryStream).
		 */
		inline std::streamsize Read(Ty& outData) noexcept;

		template <typename Ty>
			requires (!RestrictRead && (std::is_trivially_copyable_v<Ty> || HasLoadFunc<RestrictWrite, Ty>))
		/* Reads sizeof(@Ty) * @expectedElements bytes from the stream and reinterprets them as type @Ty for every element.
		 * If @expectedElements is 0, reads sizeof(@Ty) * @outData.capacity() bytes instead.
		 * Returns the number of bytes read.
		 * Returns -1 if !IsInput(), the stream is in an invalid state, or if both @expectedElements and @outData.capacity() are 0.
		 * If EOF is reached, returns the number of bytes read from the partial read.
		 * Requires RestrictRead to be false (CMBinaryStreamInput or CMBinaryStream<false, ...>)
		 *   and type @Ty to be trivially copyable, or has defined the member function
		 *   @outData.Load(const CMBinaryStream<false, false>& binaryStream).
		 */
		inline std::streamsize Read(std::vector<Ty>& outData, size_t expectedElements = 0) noexcept;

		/* Reads sizeof(char) * @expectedLength bytes from the stream into @outStr.
		 * if @expectedLength is 0, reads sizeof(char) * @outStr.capacity().
		 * Returns the number of bytes read.
		 * Returns -1 if !IsInput(), the stream is in an invalid state, or if @expectedLength and @outStr.capacity() are 0.
		 * If EOF is reached, returns the number of bytes read from the partial read.
		 * Requires RestrictRead to be false (CMBinaryStreamInput or CMBinaryStream<false, ...>).
		 */
		inline std::streamsize Read(std::string& outStr, size_t expectedLength = 0) noexcept
			requires (!RestrictRead);

		/* Read @numBytes from the stream into @pData.
		 * Returns the number of bytes read from the stream.
		 * Returns -1 if !IsInput(), @pData is nullptr, or @numBytes <= 0.
		 * Requires
		 */
		inline [[nodiscard]] std::streamsize ReadRaw(void* pData, std::streamsize numBytes) noexcept
			requires (!RestrictRead);

		inline [[nodiscard]] bool IsOpen() const noexcept;
		inline [[nodiscard]] bool IsClosed() const noexcept;

		inline [[nodiscard]] bool IsInput() const noexcept;
		inline [[nodiscard]] bool IsOutput() const noexcept;

		inline [[nodiscard]] bool IsEOF() const noexcept;

		inline [[nodiscard]] bool IsValidState() const noexcept;
		inline [[nodiscard]] bool IsValidStreamType() const noexcept;
		inline [[nodiscard]] bool IsValidFileState() const noexcept;

		inline [[nodiscard]] CMBinaryStreamType Type() const noexcept { return m_CurrentStreamType; }
		inline [[nodiscard]] CMBinaryStreamFileStateType FileState() const noexcept { return m_CurrentFileState; }
		inline [[nodiscard]] CMBinaryStreamOperationType LastOperation() const noexcept { return m_LastOperation; }
		inline [[nodiscard]] const std::filesystem::path& CurrentFilePath() const noexcept { return m_CurrentFilePath; }
		inline [[nodiscard]] const std::string& CurrentFileName() const noexcept { return m_CurrentFileName; }
		inline [[nodiscard]] uintmax_t CurrentFileSizeBytes() const noexcept { return m_CurrentFileSizeBytes; }
		inline [[nodiscard]] std::streamsize CurrentFileGetPosBytes() const noexcept { return m_CurrentFileGetPosBytes; }
		inline [[nodiscard]] std::streamsize CurrentFilePutPosBytes() const noexcept { return m_CurrentFilePutPosBytes; }
		inline [[nodiscard]] int CurrentStreamFlags() const noexcept { return m_CurrentStreamFlags; }

		inline [[nodiscard]] operator bool() const noexcept;

		template <typename Ty>
			requires (!RestrictWrite && (std::is_trivially_copyable_v<Ty> || HasSaveFunc<RestrictRead, Ty>))
		/* Matching overload for Write(const Ty& data). */
		inline bool operator<<(const Ty& rhs) noexcept;

		template <typename Ty>
			requires (!RestrictWrite && std::is_trivially_copyable_v<Ty>)
		/* Matching overload for Write(const std::vector<Ty>& data). */
		inline bool operator<<(const std::vector<Ty>& rhs) noexcept;

		/* Matching overload for Write(const std::string& str). */
		inline bool operator<<(const std::string& rhs) noexcept
			requires (!RestrictWrite);

		template <typename Ty>
			requires (!RestrictRead && (std::is_trivially_copyable_v<Ty> || HasLoadFunc<RestrictWrite, Ty>))
		/* Matching overload for Read(Ty& outData). */
		inline std::streamsize operator>>(Ty& rhs) noexcept;
		
		/* No matching overloads for Read(std::vector<Ty>& outData, size_t expectedElements)
		 *  or Read(std::string& outStr, size_t expectedLength) due to them requiring lengths
		 *  while operator>> can only take one argument.
		 */
	private:
		CMBinaryStreamType m_CurrentStreamType = CMBinaryStreamType::INVALID;
		CMBinaryStreamFileStateType m_CurrentFileState = CMBinaryStreamFileStateType::INVALID;
		CMBinaryStreamOperationType m_LastOperation = CMBinaryStreamOperationType::INVALID;
		std::filesystem::path m_CurrentFilePath;
		std::string m_CurrentFileName;
		uintmax_t m_CurrentFileSizeBytes = 0;
		std::streamsize m_CurrentFileGetPosBytes = -1;
		std::streamsize m_CurrentFilePutPosBytes = -1;
		int m_CurrentStreamFlags = -1; /* std::ios::binary, std::ios::in, std::ios::out, std::ios::trunc... */
		std::fstream m_Stream;
	};

	class CMBinaryStreamInput : public CMBinaryStream<false, true>
	{
	public:
		using Parent = CMBinaryStream<false, true>;

		CMBinaryStreamInput(const std::filesystem::path& filePathRef) noexcept;
		CMBinaryStreamInput() = default;
		~CMBinaryStreamInput() = default;
	};

	class CMBinaryStreamOutput : public CMBinaryStream<true, false>
	{
	public:
		using Parent = CMBinaryStream<true, false>;

		CMBinaryStreamOutput(const std::filesystem::path& filePathRef) noexcept;
		CMBinaryStreamOutput() = default;
		~CMBinaryStreamOutput() = default;
	};

	class CMBinaryStreamIO : public CMBinaryStream<false, false>
	{
	public:
		using Parent = CMBinaryStream<false, false>;

		CMBinaryStreamIO(const std::filesystem::path& filePathRef) noexcept;
		CMBinaryStreamIO() = default;
		~CMBinaryStreamIO() = default;
	};

#pragma region Stream Management
	template <bool RestrictRead, bool RestrictWrite>
		requires (!(RestrictRead && RestrictWrite))
	inline CMBinaryStream<RestrictRead, RestrictWrite>::CMBinaryStream(CMBinaryStreamType streamType, const std::filesystem::path& filePath) noexcept
	{
		Set(streamType, filePath);
		Open();
	}

	template <bool RestrictRead, bool RestrictWrite>
		requires (!(RestrictRead && RestrictWrite))
	inline [[nodiscard]] bool CMBinaryStream<RestrictRead, RestrictWrite>::Set(CMBinaryStreamType streamType, const std::filesystem::path& filePath) noexcept
	{
		Close();

		if (Utility::Assert(streamType != CMBinaryStreamType::INVALID))
			return false;

		if constexpr (RestrictRead)
			if (Utility::Assert(streamType != CMBinaryStreamType::READ))
				return false;
		else if constexpr (RestrictWrite)
			if (Utility::Assert(streamType != CMBinaryStreamType::WRITE))
				return false;

		m_CurrentStreamType = streamType;

		if (filePath.empty())
			return false;
		else if (!std::filesystem::exists(filePath))
		{
			m_CurrentFileState = CMBinaryStreamFileStateType::ABSENT;
			return false;
		}

		m_CurrentFileState = CMBinaryStreamFileStateType::SET;
		m_LastOperation = CMBinaryStreamOperationType::SET;
		m_CurrentFilePath = filePath;
		m_CurrentFileName = m_CurrentFilePath.string();

		m_CurrentStreamFlags = std::ios::binary;

		if (IsInput())
			m_CurrentStreamFlags |= std::ios::in;
		if (IsOutput())
			m_CurrentStreamFlags |= (std::ios::out | std::ios::trunc);

		return true;
	}

	template <bool RestrictRead, bool RestrictWrite>
		requires (!(RestrictRead && RestrictWrite))
	inline void CMBinaryStream<RestrictRead, RestrictWrite>::Open() noexcept
	{
		if (m_Stream.is_open() || !IsValidState())
			return;

		m_Stream.open(m_CurrentFileName, m_CurrentStreamFlags);

		if (!m_Stream)
			return;

		m_CurrentFileGetPosBytes = 0;
		m_CurrentFilePutPosBytes = 0;
		m_CurrentFileSizeBytes = std::filesystem::file_size(m_CurrentFilePath);

		m_CurrentFileState = CMBinaryStreamFileStateType::TARGETED;
		m_LastOperation = CMBinaryStreamOperationType::OPEN;
	}

	template <bool RestrictRead, bool RestrictWrite>
		requires (!(RestrictRead && RestrictWrite))
	inline void CMBinaryStream<RestrictRead, RestrictWrite>::Close() noexcept
	{
		if (!m_Stream.is_open())
			return;

		/* If m_Stream encountered an error before calling Close(), the stream
		 * might be in a bad state and flush() or close() might silently no-op.
		 * Maybe.
		 */
		if (!m_Stream.good())
			m_Stream.clear();

		if constexpr (!RestrictWrite)
			if (m_LastOperation == CMBinaryStreamOperationType::WRITE)
				m_Stream.flush();

		m_Stream.close();
		m_Stream.clear();

		m_CurrentFileState = CMBinaryStreamFileStateType::UNTARGETED;
		m_LastOperation = CMBinaryStreamOperationType::CLOSE;

		m_CurrentFileGetPosBytes = -1;
		m_CurrentFilePutPosBytes = -1;
		m_CurrentFileSizeBytes = 0;
	}

	template <bool RestrictRead, bool RestrictWrite>
		requires (!(RestrictRead && RestrictWrite))
	inline bool CMBinaryStream<RestrictRead, RestrictWrite>::Flush() noexcept
		requires (!RestrictWrite)
	{
		if (!IsValidState())
			return false;

		m_Stream.flush();

		m_LastOperation = CMBinaryStreamOperationType::FLUSH;
		return m_Stream.good();
	}

	template <bool RestrictRead, bool RestrictWrite>
		requires (!(RestrictRead&& RestrictWrite))
	inline bool CMBinaryStream<RestrictRead, RestrictWrite>::Clear() noexcept
	{
		if (m_Stream.good())
			return true;

		m_Stream.clear();

		m_LastOperation = CMBinaryStreamOperationType::CLEAR;
		return m_Stream.good();
	}

	template <bool RestrictRead, bool RestrictWrite>
		requires (!(RestrictRead && RestrictWrite))
	inline bool CMBinaryStream<RestrictRead, RestrictWrite>::SeekBeginningIn() noexcept
		requires (!RestrictRead)
	{
		if (!IsValidState())
			return false;

		m_Stream.seekg(std::ios::beg);

		/* Both get and put positions are set due to std::fstream implicitly handling the synchronization of both pointers. */
		m_CurrentFileGetPosBytes = m_Stream.tellg();
		m_CurrentFilePutPosBytes = m_Stream.tellp();

		m_LastOperation = CMBinaryStreamOperationType::SEEK_BEGINNING_IN;

		return m_Stream.good();
	}

	template <bool RestrictRead, bool RestrictWrite>
		requires (!(RestrictRead && RestrictWrite))
	inline bool CMBinaryStream<RestrictRead, RestrictWrite>::SeekBeginningOut() noexcept
		requires (!RestrictWrite)
	{
		if (!IsValidState())
			return false;

		m_Stream.seekp(std::ios::beg);

		/* Both get and put positions are set due to std::fstream implicitly handling the synchronization of both pointers. */
		m_CurrentFileGetPosBytes = m_Stream.tellg();
		m_CurrentFilePutPosBytes = m_Stream.tellp();

		m_LastOperation = CMBinaryStreamOperationType::SEEK_BEGINNING_OUT;

		return m_Stream.good();
	}

	template <bool RestrictRead, bool RestrictWrite>
		requires (!(RestrictRead&& RestrictWrite))
	inline bool CMBinaryStream<RestrictRead, RestrictWrite>::SeekIn(std::streamsize bytePos) noexcept
		requires (!RestrictRead)
	{
		std::streamsize currentFileByteSize = static_cast<std::streamsize>(m_CurrentFileSizeBytes);

		if (bytePos < 0 || bytePos >= currentFileByteSize || !IsValidState())
			return false;

		m_Stream.seekg(bytePos);

		/* Both get and put positions are set due to std::fstream implicitly handling the synchronization of both pointers. */
		m_CurrentFileGetPosBytes = m_Stream.tellg();
		m_CurrentFilePutPosBytes = m_Stream.tellp();

		m_LastOperation = CMBinaryStreamOperationType::SEEK_IN;

		return m_Stream.good();
	}
#pragma endregion

#pragma region Stream Output
	template <bool RestrictRead, bool RestrictWrite>
		requires (!(RestrictRead && RestrictWrite))
	template <typename Ty>
		requires (!RestrictWrite && (std::is_trivially_copyable_v<Ty> || HasSaveFunc<RestrictRead, Ty>))
	inline bool CMBinaryStream<RestrictRead, RestrictWrite>::Write(const Ty& data) noexcept
	{
		if (!IsOutput() || !IsValidState())
			return false;

		if constexpr (!std::is_trivially_copyable_v<Ty>)
			return data.Save(*this);
		else
			return WriteRaw(&data, sizeof(Ty));
	}

	template <bool RestrictRead, bool RestrictWrite>
		requires (!(RestrictRead && RestrictWrite))
	template <typename Ty>
		requires (!RestrictWrite && (std::is_trivially_copyable_v<Ty> || HasSaveFunc<RestrictRead, Ty>))
	inline bool CMBinaryStream<RestrictRead, RestrictWrite>::Write(const std::vector<Ty>& data) noexcept
	{
		if (!IsOutput() || !IsValidState() || data.empty())
			return false;

		if constexpr (!std::is_trivially_copyable_v<Ty>)
		{
			for (const Ty& element : data)
				if (!element.Save(*this))
					return false;

			return m_Stream.good();
		}

		std::streamsize numBytes = static_cast<std::streamsize>(data.size() * sizeof(Ty));
		return WriteRaw(data.data(), numBytes);
	}

	template <bool RestrictRead, bool RestrictWrite>
		requires (!(RestrictRead && RestrictWrite))
	inline bool CMBinaryStream<RestrictRead, RestrictWrite>::Write(const std::string& str) noexcept
		requires (!RestrictWrite)
	{
		if (!IsOutput() || !IsValidState() || str.empty())
			return false;

		std::streamsize numBytes = static_cast<std::streamsize>(str.length() * sizeof(char));
		return WriteRaw(str.data(), numBytes);
	}

	template <bool RestrictRead, bool RestrictWrite>
		requires (!(RestrictRead && RestrictWrite))
	inline bool CMBinaryStream<RestrictRead, RestrictWrite>::WriteRaw(const void* pData, std::streamsize numBytes) noexcept
		requires (!RestrictWrite)
	{
		if (!IsOutput() || !IsValidState() || pData == nullptr || numBytes <= 0)
			return false;

		m_Stream.write(reinterpret_cast<const char*>(pData), numBytes);

		m_LastOperation = CMBinaryStreamOperationType::WRITE;

		/* Both get and put positions are set due to std::fstream implicitly handling the synchronization of both pointers. */
		m_CurrentFileGetPosBytes = m_Stream.tellg();
		m_CurrentFilePutPosBytes = m_Stream.tellp();

		m_CurrentFileSizeBytes = static_cast<uintmax_t>(m_CurrentFilePutPosBytes);
		return m_Stream.good();
	}
#pragma endregion

#pragma region Stream Input
	template <bool RestrictRead, bool RestrictWrite>
		requires (!(RestrictRead && RestrictWrite))
	template <typename Ty>
		requires (!RestrictRead && (std::is_trivially_copyable_v<Ty> || HasLoadFunc<RestrictWrite, Ty>))
	inline std::streamsize CMBinaryStream<RestrictRead, RestrictWrite>::Read(Ty& outData) noexcept
	{
		if (!IsInput() || !IsValidState())
			return -1;
	
		if constexpr (!std::is_trivially_copyable_v<Ty>)
			return outData.Load(*this);
		else
			return ReadRaw(&outData, sizeof(Ty));
	}

	template <bool RestrictRead, bool RestrictWrite>
		requires (!(RestrictRead && RestrictWrite))
	template <typename Ty>
		requires (!RestrictRead && (std::is_trivially_copyable_v<Ty> || HasLoadFunc<RestrictWrite, Ty>))
	inline std::streamsize CMBinaryStream<RestrictRead, RestrictWrite>::Read(std::vector<Ty>& outData, size_t expectedElements) noexcept
	{
		if (!IsInput() || !IsValidState() || (expectedElements == 0 && outData.empty()))
			return -1;

		size_t numElementsToRead = expectedElements == 0 ?
			outData.capacity() : expectedElements;

		outData.resize(numElementsToRead);
		
		if constexpr (!std::is_trivially_copyable_v<Ty>)
		{
			std::streamsize bytesRead = 0;

			for (Ty& element : outData)
				bytesRead += element.Load(*this);

			return bytesRead;
		}

		std::streamsize numBytes = static_cast<std::streamsize>(numElementsToRead * sizeof(Ty));
		return ReadRaw(outData.data(), numBytes);
	}

	template <bool RestrictRead, bool RestrictWrite>
		requires (!(RestrictRead&& RestrictWrite))
	inline std::streamsize CMBinaryStream<RestrictRead, RestrictWrite>::Read(std::string& outStr, size_t expectedLength) noexcept
		requires (!RestrictRead)
	{
		if (!IsInput() || !IsValidState() || (expectedLength == 0 && outStr.empty()))
			return -1;

		size_t numCharsToRead = expectedLength == 0 ?
			outStr.capacity() : expectedLength;

		outStr.resize(numCharsToRead);
		
		std::streamsize numBytes = static_cast<std::streamsize>(numCharsToRead * sizeof(char));
		return ReadRaw(outStr.data(), numBytes);
	}

	template <bool RestrictRead, bool RestrictWrite>
		requires (!(RestrictRead&& RestrictWrite))
	inline [[nodiscard]] std::streamsize CMBinaryStream<RestrictRead, RestrictWrite>::ReadRaw(void* pData, std::streamsize numBytes) noexcept
		requires (!RestrictRead)
	{
		std::streamsize byteOffsetFromCurrent = m_CurrentFileGetPosBytes + numBytes;
		std::streamsize currentFileSizeBytes = static_cast<std::streamsize>(m_CurrentFileSizeBytes);

		if (byteOffsetFromCurrent >= currentFileSizeBytes ||
			pData == nullptr ||
			numBytes <= 0 ||
			!IsInput() ||
			!IsValidState())
			return -1;

		m_Stream.read(reinterpret_cast<char*>(pData), numBytes);

		m_LastOperation = CMBinaryStreamOperationType::READ;

		/* Both get and put positions are set due to std::fstream implicitly handling the synchronization of both pointers. */
		m_CurrentFileGetPosBytes = m_Stream.tellg();
		m_CurrentFilePutPosBytes = m_Stream.tellp();

		/* Return the number of bytes read. */
		return m_Stream.gcount();
	}
#pragma endregion

#pragma region Stream State
	template <bool RestrictRead, bool RestrictWrite>
		requires (!(RestrictRead && RestrictWrite))
	inline [[nodiscard]] bool CMBinaryStream<RestrictRead, RestrictWrite>::IsOpen() const noexcept
	{
		return m_Stream.is_open();
	}

	template <bool RestrictRead, bool RestrictWrite>
		requires (!(RestrictRead && RestrictWrite))
	inline [[nodiscard]] bool CMBinaryStream<RestrictRead, RestrictWrite>::IsClosed() const noexcept
	{
		return !m_Stream.is_open();
	}

	template <bool RestrictRead, bool RestrictWrite>
		requires (!(RestrictRead && RestrictWrite))
	inline [[nodiscard]] bool CMBinaryStream<RestrictRead, RestrictWrite>::IsInput() const noexcept
	{
		if constexpr (RestrictRead)
			return false;
		else
			return m_CurrentStreamType == CMBinaryStreamType::READ ||
				m_CurrentStreamType == CMBinaryStreamType::READ_WRITE;
	}

	template <bool RestrictRead, bool RestrictWrite>
		requires (!(RestrictRead && RestrictWrite))
	inline [[nodiscard]] bool CMBinaryStream<RestrictRead, RestrictWrite>::IsOutput() const noexcept
	{
		if constexpr (RestrictWrite)
			return false;
		else
			return m_CurrentStreamType == CMBinaryStreamType::WRITE ||
				m_CurrentStreamType == CMBinaryStreamType::READ_WRITE;
	}

	template <bool RestrictRead, bool RestrictWrite>
		requires (!(RestrictRead && RestrictWrite))
	inline [[nodiscard]] bool CMBinaryStream<RestrictRead, RestrictWrite>::IsEOF() const noexcept
	{
		return m_Stream.eof();
	}

	template <bool RestrictRead, bool RestrictWrite>
		requires (!(RestrictRead && RestrictWrite))
	inline [[nodiscard]] bool CMBinaryStream<RestrictRead, RestrictWrite>::IsValidState() const noexcept
	{
		return m_Stream.rdstate() == std::ios::goodbit &&
			IsValidStreamType() &&
			IsValidFileState();
	}

	template <bool RestrictRead, bool RestrictWrite>
		requires (!(RestrictRead && RestrictWrite))
	inline [[nodiscard]] bool CMBinaryStream<RestrictRead, RestrictWrite>::IsValidStreamType() const noexcept
	{
		bool isMismatchingStreamType = false;

		if constexpr (RestrictRead)
			isMismatchingStreamType = m_CurrentStreamType == CMBinaryStreamType::READ;
		else if constexpr (RestrictWrite)
			if (!isMismatchingStreamType)
				isMismatchingStreamType = m_CurrentStreamType == CMBinaryStreamType::WRITE;

		return m_CurrentStreamType != CMBinaryStreamType::INVALID &&
			!isMismatchingStreamType;
	}

	template <bool RestrictRead, bool RestrictWrite>
		requires (!(RestrictRead && RestrictWrite))
	inline [[nodiscard]] bool CMBinaryStream<RestrictRead, RestrictWrite>::IsValidFileState() const noexcept
	{
		return m_CurrentFileState != CMBinaryStreamFileStateType::INVALID &&
			m_CurrentFileState != CMBinaryStreamFileStateType::ABSENT;
	}
#pragma endregion

#pragma region Stream Operators
	template <bool RestrictRead, bool RestrictWrite>
		requires (!(RestrictRead && RestrictWrite))
	inline [[nodiscard]] CMBinaryStream<RestrictRead, RestrictWrite>::operator bool() const noexcept
	{
		return m_Stream.good() && IsValidState();
	}

	template <bool RestrictRead, bool RestrictWrite>
		requires (!(RestrictRead && RestrictWrite))
	template <typename Ty>
		requires (!RestrictWrite && (std::is_trivially_copyable_v<Ty> || HasSaveFunc<RestrictRead, Ty>))
	inline bool CMBinaryStream<RestrictRead, RestrictWrite>::operator<<(const Ty& rhs) noexcept
	{
		return Write(rhs);
	}

	template <bool RestrictRead, bool RestrictWrite>
		requires (!(RestrictRead && RestrictWrite))
	template <typename Ty>
		requires (!RestrictWrite && std::is_trivially_copyable_v<Ty>)
	inline bool CMBinaryStream<RestrictRead, RestrictWrite>::operator<<(const std::vector<Ty>& rhs) noexcept
	{
		return Write(rhs);
	}

	template <bool RestrictRead, bool RestrictWrite>
		requires (!(RestrictRead && RestrictWrite))
	inline bool CMBinaryStream<RestrictRead, RestrictWrite>::operator<<(const std::string& rhs) noexcept
		requires (!RestrictWrite)
	{
		return Write(rhs);
	}

	template <bool RestrictRead, bool RestrictWrite>
		requires (!(RestrictRead && RestrictWrite))
	template <typename Ty>
		requires (!RestrictRead && (std::is_trivially_copyable_v<Ty> || HasLoadFunc<RestrictWrite, Ty>))
	inline std::streamsize CMBinaryStream<RestrictRead, RestrictWrite>::operator>>(Ty& rhs) noexcept
	{
		return Read(rhs);
	}
#pragma endregion
}