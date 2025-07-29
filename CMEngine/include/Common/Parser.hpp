#pragma once

#include <string>
#include <concepts>
#include <limits>
#include <type_traits>
#include <cstdint>

namespace CMEngine::Common::Parse
{
	template <typename>
	inline constexpr bool DependentFalse = false;

	template <typename Ty>
	concept ParsableOutput = std::integral<Ty> || std::floating_point<Ty>;

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

	enum class ParseResultType : uint8_t
	{
		INVALID,
		SUCCEEDED,
		FAILED_INVALID_FORMAT,
		FAILED_EMPTY_STRING,
		FAILED_NON_NUMERIC,
		FAILED_NON_NUMERIC_EXTRANEOUS_CHARACTERS,
		FAILED_SIGN_MISMATCH,
		FAILED_VALUE_OVERFLOW
	};

	struct ParseResult
	{
		inline constexpr ParseResult(ParseResultType result) noexcept;

		inline static constexpr [[nodiscard]] bool IsSucceededResult(ParseResultType type) noexcept;
		inline static constexpr [[nodiscard]] bool IsFailedResult(ParseResultType type) noexcept;
		inline static constexpr [[nodiscard]] std::string_view ResultToString(ParseResultType type) noexcept;
		inline static constexpr [[nodiscard]] std::wstring_view ResultToWString(ParseResultType type) noexcept;

		inline constexpr [[nodiscard]] bool Succeeded() const noexcept;
		inline constexpr [[nodiscard]] bool Failed() const noexcept;
		inline constexpr [[nodiscard]] const std::string_view ToString() const noexcept;
		inline constexpr [[nodiscard]] const std::wstring_view ToWString() const noexcept;

		ParseResultType Result = ParseResultType::INVALID;
	};

	class Parser
	{
	public:
		Parser() = default;
		~Parser() = default;
	public:
		template <typename T>
			requires std::is_arithmetic_v<T>
		inline [[nodiscard]] ParseResult TryParseNumericIntegral(const std::string& str, T& outValue) const noexcept;

		/* Assigns a substring of @to with leading whitespaces removed from @from.
		 * Returns true if any leading whitespaces were found and trimmed.
		 */
		inline [[nodiscard]] bool TrimLeadingWhitespace(const std::string& from, std::string& to) const noexcept;
	private:
	};

	inline constexpr ParseResult::ParseResult(ParseResultType result) noexcept
		: Result(result)
	{
	}

	inline constexpr [[nodiscard]] bool ParseResult::IsSucceededResult(ParseResultType type) noexcept
	{
		return type == ParseResultType::SUCCEEDED;
	}

	inline constexpr [[nodiscard]] bool ParseResult::IsFailedResult(ParseResultType type) noexcept
	{
		return static_cast<uint8_t>(type) > static_cast<uint8_t>(ParseResultType::SUCCEEDED);
	}

	inline constexpr [[nodiscard]] std::string_view ParseResult::ResultToString(ParseResultType type) noexcept
	{
		switch (type)
		{
		case ParseResultType::INVALID:
			return std::string_view("INVALID");
		case ParseResultType::SUCCEEDED:
			return std::string_view("SUCCEEDED");
		case ParseResultType::FAILED_INVALID_FORMAT:
			return std::string_view("FAILED_INVALID_FORMAT");
		case ParseResultType::FAILED_EMPTY_STRING:
			return std::string_view("FAILED_EMPTY_STRING");
		case ParseResultType::FAILED_NON_NUMERIC:
			return std::string_view("FAILED_NON_NUMERIC");
		case ParseResultType::FAILED_NON_NUMERIC_EXTRANEOUS_CHARACTERS:
			return std::string_view("FAILED_NON_NUMERIC_EXTRANEOUS_CHARACTERS");
		case ParseResultType::FAILED_SIGN_MISMATCH:
			return std::string_view("FAILED_SIGN_MISMATCH");
		case ParseResultType::FAILED_VALUE_OVERFLOW:
			return std::string_view("FAILED_VALUE_OVERFLOW");
		default:
			return std::string_view("UNKNOWN");
		}
	}

	inline constexpr [[nodiscard]] std::wstring_view ParseResult::ResultToWString(ParseResultType type) noexcept
	{
		switch (type)
		{
		case ParseResultType::INVALID:
			return std::wstring_view(L"INVALID");
		case ParseResultType::SUCCEEDED:
			return std::wstring_view(L"SUCCEEDED");
		case ParseResultType::FAILED_INVALID_FORMAT:
			return std::wstring_view(L"FAILED_INVALID_FORMAT");
		case ParseResultType::FAILED_EMPTY_STRING:
			return std::wstring_view(L"FAILED_EMPTY_STRING");
		case ParseResultType::FAILED_NON_NUMERIC:
			return std::wstring_view(L"FAILED_NON_NUMERIC");
		case ParseResultType::FAILED_NON_NUMERIC_EXTRANEOUS_CHARACTERS:
			return std::wstring_view(L"FAILED_NON_NUMERIC_EXTRANEOUS_CHARACTERS");
		case ParseResultType::FAILED_SIGN_MISMATCH:
			return std::wstring_view(L"FAILED_SIGN_MISMATCH");
		case ParseResultType::FAILED_VALUE_OVERFLOW:
			return std::wstring_view(L"FAILED_VALUE_OVERFLOW");
		default:
			return std::wstring_view(L"UNKNOWN");
		}
	}

	inline constexpr [[nodiscard]] bool ParseResult::Succeeded() const noexcept
	{
		return IsSucceededResult(Result);
	}

	inline constexpr [[nodiscard]] bool ParseResult::Failed() const noexcept
	{
		return IsFailedResult(Result);
	}

	inline constexpr [[nodiscard]] const std::string_view ParseResult::ToString() const noexcept
	{
		return ResultToString(Result);
	}

	inline constexpr [[nodiscard]] const std::wstring_view ParseResult::ToWString() const noexcept
	{
		return ResultToWString(Result);
	}

	template <typename T>
		requires std::is_arithmetic_v<T>
	inline [[nodiscard]] ParseResult Parser::TryParseNumericIntegral(const std::string& str, T& outValue) const noexcept
	{
		if (str.length() == 0)
			return ParseResult(ParseResultType::FAILED_EMPTY_STRING);

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
				return ParseResult(ParseResultType::FAILED_SIGN_MISMATCH);

		if (!std::isdigit(firstChar) && firstChar != '-' && firstChar != '+')
			return ParseResult(ParseResultType::FAILED_NON_NUMERIC);

		try
		{
			if constexpr (std::unsigned_integral<T>)
				parsedValue = std::stoull(parseInput, &index);
			else if constexpr (std::signed_integral<T>)
				parsedValue = std::stoll(parseInput, &index);
			else if constexpr (std::floating_point<T>)
				parsedValue = std::stold(parseInput, &index);
			else
				static_assert(DependentFalse<T>, "Invalid Type!!! Panic!!!!!");
		}
		catch (...)
		{
			return ParseResult(ParseResultType::FAILED_INVALID_FORMAT);
		}

		if (index != parseInput.length())
			return ParseResult(ParseResultType::FAILED_NON_NUMERIC_EXTRANEOUS_CHARACTERS);

		if constexpr (!std::is_same_v<T, ParseType>)
			if (parsedValue > static_cast<ParseType>(std::numeric_limits<T>::max()))
				return ParseResult(ParseResultType::FAILED_VALUE_OVERFLOW);

		outValue = static_cast<T>(parsedValue);
		return ParseResult(ParseResultType::SUCCEEDED);
	}
	
	/* Fix bug around whitespace index... (only whitespaces, etc) */
	inline [[nodiscard]] bool Parser::TrimLeadingWhitespace(const std::string& from, std::string& to) const noexcept
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