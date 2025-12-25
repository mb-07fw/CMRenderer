#pragma once

#include "Asset/Asset.hpp"
#include "Event/Observer.hpp"
#include "Types.hpp"
#include "Math.hpp"

namespace CMEngine
{
	struct CameraData
	{
		inline CameraData(
			const Float3& origin,
			const Float3& lookAtPos,
			float aspect,
			float fovAngle,
			float nearZ,
			float farZ
		) noexcept
			: Origin(origin),
			LookAtPos(lookAtPos),
			Aspect(aspect),
			FovAngle(fovAngle),
			NearZ(nearZ),
			FarZ(farZ)
		{
		}

		CameraData() = default;
		~CameraData() = default;

		Float3 Origin;
		Float3 LookAtPos;
		float Aspect = 0.0f;
		float FovAngle = 0.0f;
		float NearZ = 0.0f;
		float FarZ = 0.0f;
	};

	/* Specifically match alignment to make pixels no go *poof*... */
	struct alignas(alignof(Math::Mat4)) CameraMatrices
	{
		inline CameraMatrices(const Math::Mat4& view, const Math::Mat4& proj) noexcept
			: View(view),
			  Proj(proj)
		{
		}

		CameraMatrices() = default;
		~CameraMatrices() = default;

		Math::Mat4 View = {};
		Math::Mat4 Proj = {};
	};

	struct Component {};

	struct MaterialComponent : public Component
	{
		inline MaterialComponent(Asset::AssetID meshID) noexcept
			: ID(meshID)
		{
		}

		Asset::AssetID ID;
	};

	struct TextureComponent : public Component
	{
		inline TextureComponent(Asset::AssetID id, Resource<ITexture>&& texture) noexcept
			: ID(id)
		{
			Texture = std::move(texture);
		}

		Asset::AssetID ID;
		Resource<ITexture> Texture;
	};

	struct MeshComponent : public Component
	{
		inline MeshComponent(Asset::AssetID meshID) noexcept
			: ID(meshID)
		{
		}

		Asset::AssetID ID;
	};

	struct CameraComponent : public Component
	{
		inline CameraComponent(const CameraData& data) noexcept
			: Data(data)
		{
		}

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
		CameraComponent(CameraComponent&&) = default;
		CameraComponent& operator=(const CameraComponent&) = default;
		CameraComponent& operator=(CameraComponent&&) = default;

		~CameraComponent() = default;

		void CreateViewMatrix() noexcept;
		void CreateProjectionMatrix() noexcept;
		void UpdateMatrices() noexcept;

		void SetOrigin(Float3 origin) noexcept;
		void SetLookAtPos(Float3 lookAtPos) noexcept;
		void SetAspect(float aspect) noexcept;
		void SetFovAngle(float fovAngle) noexcept;
		void SetNearZ(float nearZ) noexcept;
		void SetFarZ(float farZ) noexcept;
		void SetClean() noexcept;

		inline [[nodiscard]] bool Dirty() const noexcept { return ViewDirty || ProjDirty; }
		
		CameraData Data;
		CameraMatrices Matrices;
		bool ViewDirty = false;
		bool ProjDirty = false;
	};

	struct TransformComponent
	{
		inline void CreateModelMatrix() noexcept { Math::TransformMatrix(ModelMatrix, Transform); }

		Transform Transform;
		Math::Mat4 ModelMatrix = Math::IdentityMatrix();
		bool Dirty = false;
	};

	using LocomotionStateUnderlying = uint8_t;
	using LocomotionFlags = LocomotionStateUnderlying;

	enum class LocomotionState : LocomotionStateUnderlying
	{
		Unknown,
		Idle,
		Walking,
		Running,
		Jumping,
		Falling
	};

	inline constexpr [[nodiscard]] LocomotionState operator|(LocomotionState lhs, LocomotionState rhs) noexcept
	{
		return static_cast<LocomotionState>((LocomotionStateUnderlying)lhs | (LocomotionStateUnderlying)rhs);
	}

	inline constexpr [[nodiscard]] LocomotionState operator&(LocomotionState lhs, LocomotionState rhs) noexcept
	{
		return static_cast<LocomotionState>((LocomotionStateUnderlying)lhs & (LocomotionStateUnderlying)rhs);
	}

	inline constexpr [[nodiscard]] LocomotionState operator~(LocomotionState type) noexcept
	{
		return static_cast<LocomotionState>(~(LocomotionStateUnderlying)type);
	}

	inline constexpr void operator|=(LocomotionState& lhs, LocomotionState rhs) noexcept
	{
		lhs = lhs | rhs;
	}

	inline constexpr void operator&=(LocomotionState& lhs, LocomotionState rhs) noexcept
	{
		lhs = lhs & rhs;
	}

	inline constexpr [[nodiscard]] LocomotionFlags ToFlags(LocomotionState type) noexcept
	{
		return (LocomotionFlags)type;
	}

	inline constexpr [[nodiscard]] bool IsMovementTypeSet(LocomotionFlags flags, LocomotionState type) noexcept
	{
		return flags & (LocomotionFlags)type;
	}

	struct LocomotionComponent
	{
		LocomotionFlags Flags = ToFlags(LocomotionState::Walking);

		Vec3 Velocity = { 0.0f, 0.0f, 0.0f };
		Vec3 Acceleration = { 0.0f, 0.0f, 0.0f };
		Vec3 InputDirection = { 0.0f, 0.0f, 0.0f };

		float WalkSpeed = 7.5f; /* m/s... */
		float RunSpeed = 10.0f; /* m/s... */
		float JumpForce = 10.0f; /* m/s... */
		float Gravity = 9.81f; /* m/s... */

		inline void SetFlag(LocomotionState state) noexcept
		{
			Flags |= (LocomotionFlags)state;
		}

		inline void ClearFlag(LocomotionState state) noexcept
		{
			Flags &= ~(LocomotionFlags)state;
		}
	};
}