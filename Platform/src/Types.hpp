#pragma once

#include "Export.hpp"

#include <cstdint>

namespace Platform
{
    enum class PLATFORM_API Byte {};

    /* Provides a non-owning view onto an existing string.
     * Used as a dll-boundary-safe substituiton for std::string_view. */
    class PLATFORM_API StringView
    {
    public:
        StringView() = default;
        ~StringView() = default;

        StringView(const char* pString) noexcept;
        StringView(const char* pString, size_t size) noexcept;

        StringView(const StringView&) = default;
        StringView(StringView&&) = default;

        [[nodiscard]] StringView& operator=(const StringView&) = default;
        [[nodiscard]] StringView& operator=(StringView&&) = default;

        [[nodiscard]] bool operator==(const StringView&) const = default;
    public:
        inline [[nodiscard]] const char* Data() const noexcept { return mP_String; }
        inline [[nodiscard]] size_t Size() const noexcept { return m_Size; }
    private:
        const char* mP_String = nullptr;
        size_t m_Size = 0; /* (does not include null terminator) */
    };

    /* Provides an owning string implementation.
     * Used as a dll-boundary-safe substituiton for std::string.
     * Allocation and deallocation are performed by the Platform DLL,
     *   therefore, it's buffer must never be freed by the client. */
    class PLATFORM_API String
    {
    public:
        String() = default;
        ~String() noexcept;

        explicit String(const char* pString) noexcept;
        String(const StringView& view) noexcept;
        String(const char* pString, size_t size) noexcept;
        String(size_t initialSize, char initialChar = ' ') noexcept;
        String(const String& other) noexcept;
        String(String&& other) noexcept;

        String& operator=(const char* pString) noexcept;
        String& operator=(const String& other) noexcept;
        String& operator=(String&& other) noexcept;

        [[nodiscard]] char& operator[](size_t index) noexcept;
        [[nodiscard]] char operator[](size_t index) const noexcept;

        inline [[nodiscard]] bool operator==(const String& other) const noexcept { return Equals(other); }
    public:
        void Resize(size_t size) noexcept;
        [[nodiscard]] bool Equals(const String& other) const noexcept;

        inline [[nodiscard]] char* Data() const noexcept { return mP_String; }
        inline [[nodiscard]] size_t Size() const noexcept { return m_Size; }

        inline [[nodiscard]] char& AtUnchecked(size_t index) noexcept { return mP_String[index]; }
        inline [[nodiscard]] char AtUnchecked(size_t index) const noexcept { return mP_String[index]; }

        inline [[nodiscard]] bool IsNull() const noexcept { return !mP_String; }
        inline [[nodiscard]] StringView View() const noexcept { return StringView(mP_String, m_Size); }
    private:
        void SetNullTerminator() noexcept;

        void Alloc(size_t size) noexcept;
        void Dealloc() noexcept;

        void EnsureSize(size_t size) noexcept;

        /* Allocates a buffer and copies the provided c-string's contents into it. */
        void Assign(const char* pString) noexcept;

        /* Copies the provided c-string's contents into the previously allocated buffer. */
        void CopyRaw(const char* pString) noexcept;

        void Copy(const String& other) noexcept;
        void Steal(String&& other) noexcept;
    private:
        char* mP_String = nullptr;
        size_t m_Size = 0; /* (does not include null terminator) */
    };
}