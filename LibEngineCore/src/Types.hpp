#pragma once

#include <cstdint>
#include <cmath>
#include <vector>
#include <span>
#include <limits>
#include <type_traits>

namespace CMEngine
{
	template <typename... Args>
	constexpr bool AllTriviallyCopyable = (std::is_trivially_copyable_v<Args>&&...);

	template <typename Ty>
	inline [[nodiscard]] std::span<Ty> ToSpan(const std::vector<Ty>& vec) noexcept
	{
		return std::span<Ty>(vec.data(), vec.size());
	}

	template <typename Ty>
	inline [[nodiscard]] std::span<std::byte> ToBytesSpan(const std::vector<Ty>& vec) noexcept
	{
		return std::span<std::byte>((std::byte*)vec.data(), vec.size() * sizeof(Ty));
	}

	inline constexpr void HashCombine(size_t& outSeed, size_t value) noexcept
	{
		constexpr size_t UnholyMagicConstantIDontUnderstandThanksChatGPT = 0x9e3779b97f4a7c15ull; // // aka 2^64...

		outSeed ^= value + UnholyMagicConstantIDontUnderstandThanksChatGPT + (outSeed << 6) + (outSeed >> 2);
	}

	inline constexpr float G_NEAR_EQUAL_FLOAT_EPSILON = 1e-4f;

	/* Note: Cannot be constexpr due to std::abs not being constexpr. */
	inline [[nodiscard]] bool IsNearEqualFloat(float x, float other, float epsilon = G_NEAR_EQUAL_FLOAT_EPSILON) noexcept
	{
		return std::abs(x - other) <= epsilon;
	}

	struct Color4
	{
		inline constexpr Color4(float r, float g, float b, float a = 1.0f) noexcept
		{
			rgba[0] = r;
			rgba[1] = g;
			rgba[2] = b;
			rgba[3] = a;
		}

		Color4() = default;
		~Color4() = default;

		inline static constexpr [[nodiscard]] Color4 Red() noexcept { return { 1.0f, 0.0f, 0.0f, 1.0f }; }
		inline static constexpr [[nodiscard]] Color4 Green() noexcept { return { 1.0f, 0.0f, 0.0f, 1.0f }; }
		inline static constexpr [[nodiscard]] Color4 Blue() noexcept { return { 1.0f, 0.0f, 0.0f, 1.0f }; }
		inline static constexpr [[nodiscard]] Color4 Black() noexcept { return { 0.0f, 0.0f, 0.0f, 1.0f }; }
		inline static constexpr [[nodiscard]] Color4 White() noexcept { return { 1.0f, 1.0f, 1.0f, 1.0f }; }

		inline constexpr [[nodiscard]] float r() const noexcept { return rgba[0]; }
		inline constexpr [[nodiscard]] float g() const noexcept { return rgba[1]; }
		inline constexpr [[nodiscard]] float b() const noexcept { return rgba[2]; }
		inline constexpr [[nodiscard]] float a() const noexcept { return rgba[3]; }

		float rgba[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	};

	template <typename Ty>
	class ViewBasic
	{
	public:
		inline ViewBasic(Ty* pData) noexcept
			: mP_Ptr(pData)
		{
		}

		ViewBasic() = default;
		~ViewBasic() = default;

		inline operator Ty* () noexcept { return mP_Ptr; }
		inline ViewBasic<Ty>& operator=(Ty* pData) noexcept { mP_Ptr = pData; return*this; }
		inline Ty* operator->() noexcept { return mP_Ptr; }

		inline [[nodiscard]] Ty* Raw() noexcept { return mP_Ptr; }
		inline [[nodiscard]] bool NonNull() const noexcept { return mP_Ptr != nullptr; }
		inline [[nodiscard]] bool Null() const noexcept { return mP_Ptr == nullptr; }
	protected:
		Ty* mP_Ptr = nullptr;
	};

	/* View represents a non-onwing pointer to a resource that is NOT meant
	 *   to be deleted by the client. */
	template <typename Ty>
	using View = ViewBasic<Ty>;

	/* ConstView represents a non-onwing pointer to a const resource that is NOT meant
	 *   to be deleted by the client. */
	template <typename Ty>
	struct ConstView : public ViewBasic<const Ty>
	{
		using Parent = ViewBasic<const Ty>;
		using Parent::Parent;

		inline ConstView(View<Ty> view) noexcept
			: Parent(view.Raw())
		{
		}

		inline ConstView& operator=(View<Ty> view) noexcept
		{
			/* Weird template specialization quirk... */
			Parent::mP_Ptr = view.Raw();
			return *this;
		}

		// prevent nullptr from hitting both overloads
		ConstView& operator=(std::nullptr_t) noexcept
		{
			this->mP_Ptr = nullptr;
			return *this;
		}

		ConstView() = default;
		~ConstView() = default;
	};

	struct Float3;

	struct Float2
	{
		inline constexpr Float2(float x, float y) noexcept;
		inline constexpr Float2(Float3 float3) noexcept;

		Float2() = default;
		~Float2() = default;

		inline constexpr [[nodiscard]] bool operator==(Float2 other) const noexcept { return IsEqual(other); }
		inline constexpr [[nodiscard]] bool operator==(float value) const noexcept { return IsEqual(value); }
		inline constexpr [[nodiscard]] bool IsEqual(Float2 other) const noexcept;
		inline constexpr [[nodiscard]] bool IsEqual(float value) const noexcept;
		inline constexpr [[nodiscard]] bool IsZero() const noexcept;
		[[nodiscard]] bool IsNearEqual(Float2 other, float epsilon = G_NEAR_EQUAL_FLOAT_EPSILON) const noexcept;
		[[nodiscard]] bool IsNearEqual(float value, float epsilon = G_NEAR_EQUAL_FLOAT_EPSILON) const noexcept;

		/* Returns the aspect ratio of the x and y components. */
		inline constexpr [[nodiscard]] float Aspect() const noexcept { return x / y; }

		/* These sequence of functions take advantage of the fact that Float2's and it's children are just floats
		 *   packed sequentially in memory, and can therefore be reinterpreted as arrays.
		 *
		 * NOTE: This is safe for any derivatives of Float2 only if they're non-virtual, and Float2 is the
		 *	       only base class, as then 'this' with Float2 as the base is guaranteed to point to the same instance as
		 *         'this' for any sub-object that extends Float2.
		 */
		inline [[nodiscard]] float* Underlying() noexcept { return reinterpret_cast<float*>(this); }
		inline [[nodiscard]] const float* Underlying() const noexcept { return reinterpret_cast<const float*>(this); }

		inline [[nodiscard]] std::span<const float, 2> Span() const noexcept { return std::span<const float, 2>(Underlying(), 2); }
		inline [[nodiscard]] std::span<float, 2> Span() noexcept { return std::span<float, 2>(Underlying(), 2); }

		float x = 0.0f, y = 0.0f;
	};

	struct Float3 : public Float2
	{
		inline constexpr Float3(float x, float y = 0.0f, float z = 0.0f) noexcept;
		inline constexpr Float3(Float2 float2, float z = 0.0f) noexcept;

		Float3() = default;
		~Float3() = default;

		inline constexpr [[nodiscard]] bool operator==(Float3 other) const noexcept { return IsEqual(other); }
		inline constexpr [[nodiscard]] bool operator==(float value) const noexcept { return IsEqual(value); }
		inline constexpr [[nodiscard]] bool IsEqual(Float3 other) const noexcept;
		inline constexpr [[nodiscard]] bool IsEqual(float value) const noexcept;
		inline constexpr [[nodiscard]] bool IsZero() const noexcept { return IsEqual(0.0f); }
		[[nodiscard]] bool IsNearEqual(Float3 other, float epsilon = G_NEAR_EQUAL_FLOAT_EPSILON) const noexcept;
		[[nodiscard]] bool IsNearEqual(float value, float epsilon = G_NEAR_EQUAL_FLOAT_EPSILON) const noexcept;

		inline [[nodiscard]] std::span<const float, 3> Data() const noexcept { return std::span<const float, 3>(Underlying(), 3); }

		float z = 0.0f;
	};

	struct Rect
	{
		inline constexpr Rect(float left, float top = 0.0f, float right = 0.0f, float bottom = 0.0f) noexcept;
		inline constexpr Rect(Float2 x, Float2 y) noexcept;

		Rect() = default;
		~Rect() = default;

		inline constexpr [[nodiscard]] bool operator==(Rect other) const noexcept { return IsEqual(other); }
		inline constexpr [[nodiscard]] bool IsEqual(Rect other) const noexcept;
		inline constexpr [[nodiscard]] bool IsZero() const noexcept;
		[[nodiscard]] bool IsNearEqual(Rect other, float epsilon = G_NEAR_EQUAL_FLOAT_EPSILON) const noexcept;

		float left = 0.0f, top = 0.0f, right = 0.0f, bottom = 0.0f;
	};

	struct Transform
	{
		inline Transform(
			const Float3& scaling,
			const Float3& rotation,
			const Float3& translation
		) noexcept;

		Transform() = default;
		~Transform() = default;

		static constexpr uint32_t S_NUM_FLOAT3 = 3;

		inline constexpr [[nodiscard]] bool operator==(const Transform& other) const noexcept { return IsEqual(other); }
		inline constexpr [[nodiscard]] bool IsEqual(const Transform& other) const noexcept;
		inline constexpr [[nodiscard]] bool IsZero() const noexcept;
		[[nodiscard]] bool IsNearEqual(const Transform& other, float epsilon = G_NEAR_EQUAL_FLOAT_EPSILON) const noexcept;

		inline [[nodiscard]] const Float3* Underlying() const noexcept { return reinterpret_cast<const Float3*>(this); }
		inline [[nodiscard]] Float3* Underlying() noexcept { return reinterpret_cast<Float3*>(this); }
		inline [[nodiscard]] std::span<const Float3, S_NUM_FLOAT3> Data() const noexcept { return std::span<const Float3, S_NUM_FLOAT3>(Underlying(), S_NUM_FLOAT3); }
		inline [[nodiscard]] std::span<Float3, S_NUM_FLOAT3> Data() noexcept { return std::span<Float3, S_NUM_FLOAT3>(Underlying(), S_NUM_FLOAT3); }

		Float3 Scaling = { 1.0f, 1.0f, 1.0f };
		Float3 Rotation;
		Float3 Translation;
	};

	struct RigidTransform
	{
		inline RigidTransform(
			const Float3& rotation,
			const Float3& translation
		) noexcept;

		RigidTransform() = default;
		~RigidTransform() = default;

		static constexpr uint32_t S_NUM_FLOAT3 = 2;

		inline constexpr [[nodiscard]] bool operator==(const RigidTransform& other) const noexcept { return IsEqual(other); }
		inline constexpr [[nodiscard]] bool IsEqual(const RigidTransform& other) const noexcept;
		inline constexpr [[nodiscard]] bool IsZero() const noexcept;
		[[nodiscard]] bool IsNearEqual(const RigidTransform& other, float epsilon = G_NEAR_EQUAL_FLOAT_EPSILON) const noexcept;

		inline [[nodiscard]] const Float3* Underlying() const noexcept { return reinterpret_cast<const Float3*>(this); }
		inline [[nodiscard]] Float3* Underlying() noexcept { return reinterpret_cast<Float3*>(this); }
		inline [[nodiscard]] std::span<const Float3, S_NUM_FLOAT3> Data() const noexcept { return std::span<const Float3, S_NUM_FLOAT3>(Underlying(), S_NUM_FLOAT3); }
		inline [[nodiscard]] std::span<Float3, S_NUM_FLOAT3> Data() noexcept { return std::span<Float3, S_NUM_FLOAT3>(Underlying(), S_NUM_FLOAT3); }

		Float3 Rotation;
		Float3 Translation;
	};

	inline constexpr Float2::Float2(float x, float y) noexcept
		: x(x), y(y)
	{
	}

	inline constexpr Float2::Float2(Float3 float3) noexcept
		: x(float3.x), y(float3.y)
	{
	}

	inline constexpr [[nodiscard]] bool Float2::IsEqual(Float2 other) const noexcept
	{
		return x == other.x &&
			y == other.y;
	}

	inline constexpr [[nodiscard]] bool Float2::IsEqual(float value) const noexcept
	{
		return x == value &&
			y == value;
	}

	inline constexpr [[nodiscard]] bool Float2::IsZero() const noexcept
	{
		return x == 0.0f &&
			y == 0.0f;
	}

	inline constexpr Float3::Float3(float x, float y, float z) noexcept
		: Float2(x, y),
		  z(z)
	{
	}

	inline constexpr Float3::Float3(Float2 float2, float z) noexcept
		: Float2(float2),
		  z(z)
	{
	}

	inline constexpr [[nodiscard]] bool Float3::IsEqual(Float3 other) const noexcept
	{
		return x == other.x &&
			y == other.y &&
			z == other.z;
	}

	inline constexpr [[nodiscard]] bool Float3::IsEqual(float value) const noexcept
	{
		return x == value &&
			y == value &&
			z == value;
	}

	inline constexpr Rect::Rect(float left, float top, float right, float bottom) noexcept
		: left(left),
		  top(top),
		  right(right),
		  bottom(bottom)
	{
	}

	inline constexpr Rect::Rect(Float2 x, Float2 y) noexcept
		: left(x.x),
		  top(y.x),
		  right(x.y),
		  bottom(y.y)
	{
	}

	inline constexpr [[nodiscard]] bool Rect::IsEqual(Rect other) const noexcept
	{
		return left == other.left &&
			top == other.top &&
			right == other.right &&
			bottom == other.bottom;
	}

	inline constexpr [[nodiscard]] bool Rect::IsZero() const noexcept
	{
		return left == 0.0f &&
			top == 0.0f &&
			right == 0.0f &&
			bottom == 0.0f;
	}

	inline Transform::Transform(
		const Float3& scaling,
		const Float3& rotation,
		const Float3& translation
	) noexcept
		: Scaling(scaling),
		  Rotation(rotation),
		  Translation(translation)
	{
	}

	inline constexpr [[nodiscard]] bool Transform::IsEqual(const Transform& other) const noexcept
	{
		return Translation == other.Translation &&
			Scaling == other.Scaling &&
			Rotation == other.Rotation;
	}

	inline constexpr [[nodiscard]] bool Transform::IsZero() const noexcept
	{
		return Translation.IsZero() &&
			Scaling.IsZero() &&
			Rotation.IsZero();
	}

	inline RigidTransform::RigidTransform(
		const Float3& rotation,
		const Float3& translation
	) noexcept
		: Rotation(rotation),
		  Translation(translation)
	{
	}

	inline constexpr [[nodiscard]] bool RigidTransform::IsEqual(const RigidTransform& other) const noexcept
	{
		return Translation == other.Translation &&
			Rotation == other.Rotation;
	}

	inline constexpr [[nodiscard]] bool RigidTransform::IsZero() const noexcept
	{
		return Translation.IsZero() &&
			Rotation.IsZero();
	}

	template <typename Ty>
	class UniquePtr
	{
		using Self = UniquePtr<Ty>;
	public:
		inline UniquePtr(Ty* ptr) noexcept;
		inline ~UniquePtr() noexcept;

		UniquePtr(const Self&) = delete;
		inline UniquePtr(Self&& other) noexcept;
	public:
		Self& operator=(const Self&) = delete;
		inline Self& operator=(Self&& other) noexcept;

		inline Ty* operator->() noexcept { return Get(); }
		inline const Ty* operator->() const noexcept { return Get(); }

		inline operator bool() const noexcept { return Get(); }

		/* Returns the underlying pointer and and releases ownership of it. */
		inline Ty* Release() noexcept;

		template <typename... Args>
		inline static Self Make(Args&&... args) noexcept;

		inline [[nodiscard]] Ty* Get() noexcept { return mP_Data; }
		inline [[nodiscard]] const Ty* Get() const noexcept { return mP_Data; }
	private:
		inline void Reset() noexcept;
	private:
		Ty* mP_Data = nullptr;
	};

	template <typename Ty>
	inline UniquePtr<Ty>::UniquePtr(Ty* ptr) noexcept
	{
		mP_Data = ptr;
	}

	template <typename Ty>
	inline UniquePtr<Ty>::~UniquePtr() noexcept
	{
		Reset();
	}

	template <typename Ty>
	inline UniquePtr<Ty>::UniquePtr(Self&& other) noexcept
	{
		Reset();

		mP_Data = other.mP_Data;
		other.mP_Data = nullptr;
	}

	template <typename Ty>
	inline UniquePtr<Ty>& UniquePtr<Ty>::operator=(Self&& other) noexcept
	{
		Reset();

		mP_Data = other.mP_Data;
		other.mP_Data = nullptr;

		return *this;
	}

	template <typename Ty>
	inline void UniquePtr<Ty>::Reset() noexcept
	{
		if (mP_Data)
			delete mP_Data;

		mP_Data = nullptr;
	}

	template <typename Ty>
	inline Ty* UniquePtr<Ty>::Release() noexcept
	{
		Ty* pTemp = mP_Data;
		mP_Data = nullptr;

		return pTemp;
	}

	template <typename Ty>
	template <typename... Args>
	inline UniquePtr<Ty> UniquePtr<Ty>::Make(Args&&... args) noexcept
	{
		return UniquePtr<Ty>(new Ty(std::forward<Args>(args)...));
	}

#pragma region SharedPtr Shenanigans
	template <typename Ty>
	struct PtrControlBlock
	{
		Ty* Ptr = nullptr;
		size_t StrongRefCount = 0;
		size_t WeakRefCount = 0;

		inline void IncrementStrong() noexcept { StrongRefCount++; }
		inline void IncrementWeak() noexcept { WeakRefCount++; }
		inline void DecrementStrong() noexcept { StrongRefCount--; }
		inline void DecrementWeak() noexcept { WeakRefCount--; }

		inline [[nodiscard]] bool IsDead() const noexcept { return StrongRefCount == 0 && WeakRefCount == 0; }
	};

	/* Forward declare here for WeakPtr. */
	template <typename Ty>
	class SharedPtr;

	template <typename Ty>
	class WeakPtr
	{
		friend class SharedPtr<Ty>;
		using ControlBlock = PtrControlBlock<Ty>;
	public:
		WeakPtr() = default;
		inline WeakPtr(const SharedPtr<Ty>& shared) noexcept;
		inline ~WeakPtr() noexcept;
	public:
		inline operator bool() const noexcept { return Exists(); }

		inline void Acquire(const SharedPtr<Ty>& shared) noexcept;
		inline void Reset() noexcept;

		inline [[nodiscard]] SharedPtr<Ty> Lock() noexcept;

		inline [[nodiscard]] bool Exists() const noexcept { return mP_Control ? mP_Control->Ptr : false; }

	private:
		inline void Release() noexcept;

		inline [[nodiscard]] ControlBlock* Control() const noexcept { return mP_Control; }
	private:
		ControlBlock* mP_Control = nullptr;
	};

	template <typename Ty>
	class SharedPtr
	{
		friend class WeakPtr<Ty>;

		using ControlBlock = PtrControlBlock<Ty>;
	public:
		inline explicit SharedPtr(Ty* ptr) noexcept;
		inline SharedPtr(const SharedPtr<Ty>& other) noexcept;
		inline SharedPtr(SharedPtr<Ty>&& other) noexcept;
		inline SharedPtr(const WeakPtr<Ty>& weak) noexcept;
		inline SharedPtr() noexcept;
		inline ~SharedPtr() noexcept;
	public:
		inline SharedPtr<Ty>& operator=(const SharedPtr<Ty>& other) noexcept;
		inline SharedPtr<Ty>& operator=(SharedPtr<Ty>&& other) noexcept;

		inline Ty* operator->() noexcept { return Get(); }
		inline const Ty* operator->() const noexcept { return Get(); }

		inline operator bool() const noexcept { return Get(); }

		inline void Assign(const SharedPtr<Ty>& other) noexcept;
		inline void Assign(SharedPtr<Ty>&& other) noexcept;

		inline void Reset() noexcept;

		/* Constructs Ty in a new SharedPtr<Ty>. */
		template <typename... Args>
		inline static SharedPtr<Ty> Make(Args&&... args) noexcept;

		inline [[nodiscard]] Ty* Get() noexcept { return mP_Control ? mP_Control->Ptr : nullptr; }
		inline [[nodiscard]] const Ty* Get() const noexcept { return mP_Control ? mP_Control->Ptr : nullptr; }
	private:
		inline void Cleanup() noexcept;

		/* Destroy's the instance of the contained pointer. z*/
		inline bool Destroy() noexcept;

		inline bool Allocate() noexcept;

		inline void InheritCopy(const SharedPtr<Ty>& other) noexcept;
		inline void InheritMove(SharedPtr<Ty>&& other) noexcept;

		inline [[nodiscard]] ControlBlock* Control() const noexcept { return mP_Control; }
	private:
		static constexpr size_t S_CONTROL_BLOCK_SIZE = sizeof(ControlBlock);
		static constexpr size_t S_TYPE_SIZE = sizeof(Ty);
		ControlBlock* mP_Control = nullptr;
	};

	template <typename Ty>
	inline WeakPtr<Ty>::WeakPtr(const SharedPtr<Ty>& shared) noexcept
	{
		Acquire(shared);
	}

	template <typename Ty>
	inline WeakPtr<Ty>::~WeakPtr() noexcept
	{
		Release();
	}

	template <typename Ty>
	inline void WeakPtr<Ty>::Acquire(const SharedPtr<Ty>& shared) noexcept
	{
		Release();

		mP_Control = shared.Control();

		if (mP_Control)
			mP_Control->IncrementWeak();
	}

	template <typename Ty>
	inline void WeakPtr<Ty>::Reset() noexcept
	{
		Release();
	}

	template <typename Ty>
	inline [[nodiscard]] SharedPtr<Ty> WeakPtr<Ty>::Lock() noexcept
	{
		return SharedPtr<Ty>(*this);
	}

	template <typename Ty>
	inline void WeakPtr<Ty>::Release() noexcept
	{
		if (!mP_Control)
			return;

		mP_Control->DecrementWeak();

		/* 0 weak and strong references; we're the sole owner of the control block. */
		if (mP_Control->IsDead())
			delete mP_Control;

		mP_Control = nullptr;
	}

	template <typename Ty>
	inline SharedPtr<Ty>::SharedPtr(Ty* ptr) noexcept
	{
		Allocate();

		mP_Control->Ptr = ptr;
	}

	template <typename Ty>
	inline SharedPtr<Ty>::SharedPtr(const SharedPtr<Ty>& other) noexcept
	{
		InheritCopy(other);
	}

	template <typename Ty>
	inline SharedPtr<Ty>::SharedPtr(SharedPtr<Ty>&& other) noexcept
	{
		InheritMove(std::move(other));
	}

	template <typename Ty>
	inline SharedPtr<Ty>::SharedPtr(const WeakPtr<Ty>& weak) noexcept
	{
		mP_Control = weak.Control();

		if (mP_Control)
			mP_Control->IncrementStrong();
	}

	template <typename Ty>
	inline SharedPtr<Ty>::SharedPtr() noexcept
	{
		Allocate();
	}

	template <typename Ty>
	inline SharedPtr<Ty>::~SharedPtr() noexcept
	{
		Cleanup();
	}

	template <typename Ty>
	inline SharedPtr<Ty>& SharedPtr<Ty>::operator=(const SharedPtr<Ty>& other) noexcept
	{
		Assign(other);
		return *this;
	}

	template <typename Ty>
	inline SharedPtr<Ty>& SharedPtr<Ty>::operator=(SharedPtr<Ty>&& other) noexcept
	{
		Assign(std::move(other));
		return *this;
	}

	template <typename Ty>
	inline void SharedPtr<Ty>::Assign(const SharedPtr<Ty>& other) noexcept
	{
		Cleanup();
		InheritCopy(other);
	}

	template <typename Ty>
	inline void SharedPtr<Ty>::Assign(SharedPtr<Ty>&& other) noexcept
	{
		Cleanup();
		InheritMove(std::move(other));
	}

	template <typename Ty>
	inline void SharedPtr<Ty>::Reset() noexcept
	{
		Cleanup();
	}

	template <typename Ty>
	template <typename... Args>
	inline SharedPtr<Ty> SharedPtr<Ty>::Make(Args&&... args) noexcept
	{
		return SharedPtr<Ty>(new Ty(std::forward<Args>(args)...));
	}

	template <typename Ty>
	inline void SharedPtr<Ty>::Cleanup() noexcept
	{
		if (!mP_Control)
			return;

		mP_Control->DecrementStrong();

		if (mP_Control->Ptr && mP_Control->StrongRefCount == 0)
		{
			Destroy();
			mP_Control->Ptr = nullptr;
		}

		/* 0 weak and strong references; were the sole owner of the control block. */
		if (mP_Control->IsDead())
			delete mP_Control;

		mP_Control = nullptr;
	}

	template <typename Ty>
	inline bool SharedPtr<Ty>::Destroy() noexcept
	{
		if (!mP_Control || !mP_Control->Ptr)
			return false;

		delete mP_Control->Ptr;
		mP_Control->Ptr = nullptr;

		return true;
	}

	template <typename Ty>
	inline bool SharedPtr<Ty>::Allocate() noexcept
	{
		mP_Control = new ControlBlock();

		if (!mP_Control)
			return false;

		mP_Control->IncrementStrong();
		return true;
	}

	template <typename Ty>
	inline void SharedPtr<Ty>::InheritCopy(const SharedPtr<Ty>& other) noexcept
	{
		mP_Control = other.mP_Control;

		if (mP_Control)
			mP_Control->IncrementStrong();
	}

	template <typename Ty>
	inline void SharedPtr<Ty>::InheritMove(SharedPtr<Ty>&& other) noexcept
	{
		mP_Control = other.mP_Control;
		other.mP_Control = nullptr;
	}
#pragma endregion
}