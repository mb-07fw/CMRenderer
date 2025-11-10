#pragma once

#include "Asset/Asset.hpp"
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
}