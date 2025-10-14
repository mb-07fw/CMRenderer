#pragma once

#include "Asset/Asset.hpp"
#include "Types.hpp"
#include "Math.hpp"

namespace CMEngine
{
	struct Component {};

	struct MeshComponent : public Component
	{
		Asset::AssetID ID;
	};

	struct SpriteComponent
	{
		Asset::Material Material;
	};

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

	struct CameraMatrices
	{
		inline CameraMatrices(const Math::Matrix& view, const Math::Matrix& proj) noexcept
			: View(view),
			  Proj(proj)
		{
		}

		CameraMatrices() = default;
		~CameraMatrices() = default;

		Math::Matrix View = {};
		Math::Matrix Proj = {};
	};

	struct CameraComponent : public Component
	{
		inline CameraComponent(const CameraData& data) noexcept
			: Data(data)
		{
		}

		inline CameraComponent(
			const Float3& origin,
			const Float3& lookAtPos,
			float aspect,
			float fovAngle,
			float nearZ,
			float farZ
		) noexcept
			: Data(
			    origin,
				lookAtPos,
				aspect,
				fovAngle,
				nearZ,
				farZ
			  )
		{
		}

		inline CameraComponent(
			const Float3& origin,
			const Float3& lookAtPos,
			float aspect,
			float fovAngle,
			float nearZ,
			float farZ,
			const Math::Matrix& viewMatrix,
			const Math::Matrix& projMatrix
		) noexcept
			: Data(
				origin,
				lookAtPos,
				aspect,
				fovAngle,
				nearZ,
				farZ
			  ),
			  Matrices(viewMatrix, projMatrix)
		{
		}

		CameraComponent() = default;
		~CameraComponent() = default;

		CameraComponent(const CameraComponent& other) = default;
		CameraComponent(CameraComponent&& other) = default;
		CameraComponent& operator=(const CameraComponent& other) = default;
		CameraComponent& operator=(CameraComponent&& other) = default;

		void CreateViewMatrix() noexcept;
		void CreateProjectionMatrix() noexcept;
		void CreateMatrices() noexcept;
		
		CameraData Data;
		CameraMatrices Matrices;
	};
}