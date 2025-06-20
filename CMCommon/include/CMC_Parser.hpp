#pragma once

#include <string>
#include <concepts>
#include <limits>
#include <type_traits>

#include <cstdint>

namespace CMCommon::Parse
{
	template <typename>
	inline constexpr bool CMDependentFalse = false;

	template <typename Ty>
	concept CMParsableOutput = std::integral<Ty> || std::floating_point<Ty>;

	template <typename Ty>
	struct VariableParsedType
	{
		using Type = std::conditional_t<
			std::floating_point<Ty>,
			long double,
			std::conditional_t<
			std::unsigned_integral<Ty>,
			uint64_t,
			int64_t
			>
		>;
	};

	enum class CMParseResultType : uint8_t
	{
		INVALID,
		SUCCEEDED,
		FAILED_INVALID_FORMAT,
		FAILED_EMPTRY_STRING,
		FAILED_NON_NUMERIC,
		FAILED_NON_NUMERIC_EXTRANEOUS_CHARACTERS,
		FAILED_SIGN_MISMATCH,
		FAILED_VALUE_OVERFLOW
	};

	struct CMParseResult
	{
		inline constexpr CMParseResult(CMParseResultType result) noexcept;

		inline static constexpr [[nodiscard]] bool IsSucceededResult(CMParseResultType type) noexcept;
		inline static constexpr [[nodiscard]] bool IsFailedResult(CMParseResultType type) noexcept;
		inline static constexpr [[nodiscard]] std::string_view ResultToString(CMParseResultType type) noexcept;
		inline static constexpr [[nodiscard]] std::wstring_view ResultToWString(CMParseResultType type) noexcept;

		inline constexpr [[nodiscard]] bool Succeeded() const noexcept;
		inline constexpr [[nodiscard]] bool Failed() const noexcept;
		inline constexpr [[nodiscard]] const std::string_view ToString() const noexcept;
		inline constexpr [[nodiscard]] const std::wstring_view ToWString() const noexcept;

		CMParseResultType Result = CMParseResultType::INVALID;
	};

	class CMParser
	{
	public:
		CMParser() = default;
		~CMParser() = default;
	public:
		template <typename T>
			requires std::is_arithmetic_v<T>
		inline [[nodiscard]] CMParseResult TryParseNumericIntegral(const std::string& str, T& outValue) const noexcept;

		/* Assigns a substring of @to with leading whitespaces removed from @from.
		 * Returns true if any leading whitespaces were found and trimmed.
		 */
		inline [[nodiscard]] bool TrimLeadingWhitespace(const std::string& from, std::string& to) const noexcept;
	private:
	};

	inline constexpr CMParseResult::CMParseResult(CMParseResultType result) noexcept
		: Result(result)
	{
	}

	inline constexpr [[nodiscard]] bool CMParseResult::IsSucceededResult(CMParseResultType type) noexcept
	{
		return type == CMParseResultType::SUCCEEDED;
	}

	inline constexpr [[nodiscard]] bool CMParseResult::IsFailedResult(CMParseResultType type) noexcept
	{
		return static_cast<uint8_t>(type) > static_cast<uint8_t>(CMParseResultType::SUCCEEDED);
	}

	inline constexpr [[nodiscard]] std::string_view CMParseResult::ResultToString(CMParseResultType type) noexcept
	{
		switch (type)
		{
		case CMParseResultType::INVALID:
			return std::string_view("INVALID");
		case CMParseResultType::SUCCEEDED:
			return std::string_view("SUCCEEDED");
		case CMParseResultType::FAILED_INVALID_FORMAT:
			return std::string_view("FAILED_INVALID_FORMAT");
		case CMParseResultType::FAILED_EMPTRY_STRING:
			return std::string_view("FAILED_EMPTRY_STRING");
		case CMParseResultType::FAILED_NON_NUMERIC:
			return std::string_view("FAILED_NON_NUMERIC");
		case CMParseResultType::FAILED_NON_NUMERIC_EXTRANEOUS_CHARACTERS:
			return std::string_view("FAILED_NON_NUMERIC_EXTRANEOUS_CHARACTERS");
		case CMParseResultType::FAILED_SIGN_MISMATCH:
			return std::string_view("FAILED_SIGN_MISMATCH");
		case CMParseResultType::FAILED_VALUE_OVERFLOW:
			return std::string_view("FAILED_VALUE_OVERFLOW");
		default:
			return std::string_view("UNKNOWN");
		}
	}

	inline constexpr [[nodiscard]] std::wstring_view CMParseResult::ResultToWString(CMParseResultType type) noexcept
	{
		switch (type)
		{
		case CMParseResultType::INVALID:
			return std::wstring_view(L"INVALID");
		case CMParseResultType::SUCCEEDED:
			return std::wstring_view(L"SUCCEEDED");
		case CMParseResultType::FAILED_INVALID_FORMAT:
			return std::wstring_view(L"FAILED_INVALID_FORMAT");
		case CMParseResultType::FAILED_EMPTRY_STRING:
			return std::wstring_view(L"FAILED_EMPTRY_STRING");
		case CMParseResultType::FAILED_NON_NUMERIC:
			return std::wstring_view(L"FAILED_NON_NUMERIC");
		case CMParseResultType::FAILED_NON_NUMERIC_EXTRANEOUS_CHARACTERS:
			return std::wstring_view(L"FAILED_NON_NUMERIC_EXTRANEOUS_CHARACTERS");
		case CMParseResultType::FAILED_SIGN_MISMATCH:
			return std::wstring_view(L"FAILED_SIGN_MISMATCH");
		case CMParseResultType::FAILED_VALUE_OVERFLOW:
			return std::wstring_view(L"FAILED_VALUE_OVERFLOW");
		default:
			return std::wstring_view(L"UNKNOWN");
		}
	}

	inline constexpr [[nodiscard]] bool CMParseResult::Succeeded() const noexcept
	{
		return IsSucceededResult(Result);
	}

	inline constexpr [[nodiscard]] bool CMParseResult::Failed() const noexcept
	{
		return IsFailedResult(Result);
	}

	inline constexpr [[nodiscard]] const std::string_view CMParseResult::ToString() const noexcept
	{
		return ResultToString(Result);
	}

	inline constexpr [[nodiscard]] const std::wstring_view CMParseResult::ToWString() const noexcept
	{
		return ResultToWString(Result);
	}

	template <typename T>
		requires std::is_arithmetic_v<T>
	inline [[nodiscard]] CMParseResult CMParser::TryParseNumericIntegral(const std::string& str, T& outValue) const noexcept
	{
		if (str.length() == 0)
			return CMParseResult(CMParseResultType::FAILED_EMPTRY_STRING);

		T value = 0;
		size_t index = 0;

		using ParseType = VariableParsedType<T>::Type;
		ParseType parsedValue = 0;

		std::string trimmedStr;
		bool isInputTrimmed = false;

		isInputTrimmed = TrimLeadingWhitespace(str, trimmedStr);

		const std::string& parseInput = isInputTrimmed ? trimmedStr : str;

		char firstChar = parseInput[0];

		/* Verify sign's match for unsigned integrals. */
		if constexpr (std::unsigned_integral<ParseType>)
			if (firstChar == '-')
				return CMParseResult(CMParseResultType::FAILED_SIGN_MISMATCH);

		if (!std::isdigit(firstChar) && firstChar != '-' && firstChar != '+')
			return CMParseResult(CMParseResultType::FAILED_NON_NUMERIC);

		try
		{
			if constexpr (std::unsigned_integral<T>)
				parsedValue = std::stoull(parseInput, &index);
			else if constexpr (std::signed_integral<T>)
				parsedValue = std::stoll(parseInput, &index);
			else if constexpr (std::floating_point<T>)
				parsedValue = std::stold(parseInput, &index);
			else
				static_assert(CMDependentFalse<T>, "Invalid Type!!! Panic!!!!!");
		}
		catch (...)
		{
			return CMParseResult(CMParseResultType::FAILED_INVALID_FORMAT);
		}

		if (index != parseInput.length())
			return CMParseResult(CMParseResultType::FAILED_NON_NUMERIC_EXTRANEOUS_CHARACTERS);

		if constexpr (!std::is_same_v<T, ParseType>)
			if (parsedValue > static_cast<ParseType>(std::numeric_limits<T>::max()))
				return CMParseResult(CMParseResultType::FAILED_VALUE_OVERFLOW);

		outValue = static_cast<T>(parsedValue);
		return CMParseResult(CMParseResultType::SUCCEEDED);
	}
	
	/* Fix bug around whitespace index... (only whitespaces, etc) */
	inline [[nodiscard]] bool CMParser::TrimLeadingWhitespace(const std::string& from, std::string& to) const noexcept
	{
		size_t indexOfFirst = 0;
		bool foundWhiteSpace = false;

		for (size_t i = 0; i < from.length(); ++i)
		{
			char at = from[i];

			if (!std::isspace(at))
				continue;
			
			foundWhiteSpace = true;
			indexOfFirst = i;
		}

		if (foundWhiteSpace)
			to = from.substr(indexOfFirst);

		return foundWhiteSpace;
	}
}