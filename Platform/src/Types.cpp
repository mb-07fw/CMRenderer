#include "Types.hpp"
#include "Common/Cast.hpp"
#include "Common/Assert.hpp"

#include <string.h> // strnlen
#include <cstring>

namespace Platform
{
    StringView::StringView(const char* pString) noexcept
    {
        constexpr size_t MaximumSize = 1024;

        mP_String = pString;
        m_Size = strnlen(pString, MaximumSize);
    }

    StringView::StringView(const char* pString, size_t size) noexcept
        : mP_String(pString),
          m_Size(size)
    {
    }

    String::String(const char* pString) noexcept
    {
        Assign(pString);
    }

    String::String(const StringView& view) noexcept
        : String(view.Data(), view.Size())
    {
    }

    String::String(const char* pString, size_t size) noexcept
    {
        Alloc(size);
        CopyRaw(pString);
    }

    String::String(size_t initialSize, char initialChar) noexcept
    {
        Alloc(initialSize);

        /* Set each character of the string to initial value... */
        std::memset(mP_String, Cast<int>(initialChar), m_Size);

        SetNullTerminator();
    }

    String::String(const String& other) noexcept
    {
        Copy(other);
    }

    String::String(String&& other) noexcept
    {
        Steal(std::move(other));
    }

    String::~String() noexcept
    {
        Dealloc();
    }

    [[nodiscard]] String& String::operator=(const char* pString) noexcept
    {
        /* GOAL: Constructor allocates specific buffer and copies string.
         *       Assignment allocates a growth factored buffer, then copies string.*/
        return *this;
    }

    [[nodiscard]] String& String::operator=(const String& other) noexcept
    {
        Copy(other);
        return *this;
    }

    [[nodiscard]] String& String::operator=(String&& other) noexcept
    {
        Steal(std::move(other));
        return *this;
    }

    void String::Resize(size_t size) noexcept
    {
        size_t geometric = m_Size + (m_Size / 2);

        size_t newSize = std::max(size, geometric);

        EnsureSize(newSize);
    }

    [[nodiscard]] bool String::Equals(const String& other) const noexcept
    {
        if ((IsNull() || other.IsNull()) ||
            m_Size != other.Size())
            return false;

        for (size_t i = 0; i < m_Size; ++i)
            if (AtUnchecked(i) != other.AtUnchecked(i))
                return false;

        return true;
    }

    [[nodiscard]] char& String::operator[](size_t index) noexcept
    {
        ASSERT(index < m_Size, "Out of bounds access.");
        return mP_String[index];
    }

    [[nodiscard]] char String::operator[](size_t index) const noexcept
    {
        ASSERT(index < m_Size, "Out of bounds access.");
        return mP_String[index];
    }

    void String::SetNullTerminator() noexcept
    {
        /* Ensure null terminator is present... */
        const size_t nullTermIndex = m_Size;
        mP_String[nullTermIndex] = '\0';
    }

    void String::Alloc(size_t size) noexcept
    {
        /* Account for null terminator... */
        const size_t allocBytes = size + 1;

        /* Allocate buffer. */
        mP_String = new char[allocBytes];
        m_Size = size;
    }

    void String::Dealloc() noexcept
    {
        if (!mP_String)
            return;
        
        delete mP_String;

        mP_String = nullptr;
        m_Size = 0;
    }

    void String::EnsureSize(size_t size) noexcept
    {
        if (size <= m_Size)
            return;

        Dealloc();
        Alloc(size);
    }

    void String::Assign(const char* pString) noexcept
    {
        constexpr size_t MaximumSize = 1024;
        size_t size = strnlen(pString, MaximumSize);

        EnsureSize(size);
        CopyRaw(pString);
    } 

    void String::CopyRaw(const char* pString) noexcept
    {
        ASSERT(mP_String, "String is not allocated.");
        ASSERT(pString, "Provided const char* is nullptr.");

        std::memcpy(mP_String, pString, m_Size);
        SetNullTerminator();
    }

    void String::Copy(const String& other) noexcept
    {
        Resize(other.Size());
        CopyRaw(other.Data());
    }

    void String::Steal(String&& other) noexcept
    {
        mP_String = other.mP_String;
        m_Size = other.m_Size;

        other.mP_String = nullptr;
        other.m_Size = 0;
    }
}
