#pragma once

#include "Platform/include/IRefCounted.hpp"
#include "Common/Assert.hpp"

namespace Engine
{
	template <typename T>
	class Ref
	{
	private:
		// this literally has no meaning. It just needs to be unique.
		// yes... this is evil, but it's also really funny.
		static constexpr uint32_t S_SuperSecretSpecialRefCountOpKey = ((2 * 16) << 26) + 43;
	public:
		Ref() = default;

		inline Ref(T* pInterface) noexcept;
		inline ~Ref() noexcept;

		inline Ref(const Ref<T>& other) noexcept;
		inline Ref(Ref<T>&& other) noexcept;

		inline Ref& operator=(const Ref<T>& other) noexcept;
		inline Ref& operator=(Ref<T>&& other) noexcept;
		inline Ref& operator=(T* pInterface) noexcept;

		inline operator T* () noexcept { return Raw(); }
		inline operator const T* () const noexcept { return Raw(); }

		inline operator T& () noexcept { return Deref(); }
		inline operator const T& () const noexcept { return Deref(); }

		inline [[nodiscard]] T& operator*() noexcept { return Deref(); }
		inline [[nodiscard]] const T& operator*() const noexcept { return Deref(); }

		inline [[nodiscard]] T* operator->() noexcept;
		inline [[nodiscard]] const T* operator->() const noexcept;

		inline [[nodiscard]] T* Raw() noexcept { return mP_Interface; }
		inline [[nodiscard]] const T* Raw() const noexcept { return mP_Interface; }

		inline [[nodiscard]] T& Deref() noexcept;
		inline [[nodiscard]] const T& Deref() const noexcept;

		inline void Reset() noexcept;
	private:
		inline void Steal(Ref<T>&& other) noexcept;
		inline void Copy(const Ref<T>& other) noexcept;
	private:
		T* mP_Interface = nullptr;
	};

	template <typename T>
	inline Ref<T>::Ref(T* pInstance) noexcept
		: mP_Interface(pInstance)
	{
		if (mP_Interface != nullptr)
			mP_Interface->AddRef(S_SuperSecretSpecialRefCountOpKey);
	}

	template <typename T>
	inline Ref<T>::~Ref() noexcept
	{
		Reset();
	}

	template <typename T>
	inline Ref<T>::Ref(const Ref<T>& other) noexcept
	{
		Copy(other);
	}

	template <typename T>
	inline Ref<T>::Ref(Ref<T>&& other) noexcept
	{
		Steal(std::forward<Ref<T>&&>(other));
	}

	template <typename T>
	inline Ref<T>& Ref<T>::operator=(const Ref<T>& other) noexcept
	{
		if (mP_Interface == other.mP_Interface)
			return *this;

		Reset();
		Copy(other);
		return *this;
	}

	template <typename T>
	inline Ref<T>& Ref<T>::operator=(Ref<T>&& other) noexcept
	{
		if (mP_Interface == other.mP_Interface)
			return *this;

		Reset();
		Steal(std::forward<Ref<T>&&>(other));
		return *this;
	}

	template <typename T>
	inline Ref<T>& Ref<T>::operator=(T* pInterface) noexcept
	{
		if (mP_Interface == pInterface)
			return *this;

		Reset();

		if (pInterface != nullptr)
			pInterface->AddRef(S_SuperSecretSpecialRefCountOpKey);

		mP_Interface = pInterface;
		return *this;
	}

	template <typename T>
	inline [[nodiscard]] T* Ref<T>::operator->() noexcept
	{
		ASSERT(mP_Interface, "(Ref<T>) Attempted to access a null Ref.");
		return mP_Interface;
	}

	template <typename T>
	inline [[nodiscard]] const T* Ref<T>::operator->() const noexcept
	{
		ASSERT(mP_Interface, "(Ref<T>) Attempted to access a null Ref.");
		return mP_Interface;
	}

	template <typename T>
	inline [[nodiscard]] T& Ref<T>::Deref() noexcept
	{
		ASSERT(mP_Interface, "(Ref<T>) Attempted to de-reference a null Ref.");
		return *mP_Interface;
	}

	template <typename T>
	inline [[nodiscard]] const T& Ref<T>::Deref() const noexcept
	{
		ASSERT(!mP_Interface, "(Ref<T>) Attempted to de-reference a null Ref.");
		return *mP_Interface;
	}

	template <typename T>
	inline void Ref<T>::Reset() noexcept
	{
		if (!mP_Interface)
			return;

		mP_Interface->Release(S_SuperSecretSpecialRefCountOpKey);
		mP_Interface = nullptr;
	}

	template <typename T>
	inline void Ref<T>::Steal(Ref<T>&& other) noexcept
	{
		mP_Interface = other.mP_Interface;
		other.mP_Interface = nullptr;
	}

	template <typename T>
	inline void Ref<T>::Copy(const Ref<T>& other) noexcept
	{
		T* pOther = other.mP_Interface;

		if (!pOther)
			return;

		pOther->AddRef(S_SuperSecretSpecialRefCountOpKey);
		mP_Interface = pOther;
	}
}