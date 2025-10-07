#pragma once

#include "Export.hpp"
#include "Platform/WinImpl/Types_WinImpl.hpp"

#include <d3d11.h>

#include <cstdint>
#include <filesystem>

namespace CMEngine::Platform::WinImpl
{

	enum class ShaderType : int8_t
	{
		INVALID = -1,
		VERTEX,
		PIXEL,
		COMPUTE
	};

	enum class AssignedShaderType : int8_t
	{
		INVALID = -1,
		QUAD_VS,
		QUAD_PS,
		GLTF_VS,
		GLTF_PS,
		CUSTOM
	};

	enum class ShaderSetType : int8_t
	{
		INVALID = -1,
		QUAD,
		GLTF,
		TOTAL_SETS
	};

	inline constexpr ShaderSetType G_IMPLEMENTED_SHADER_SETS[] = {
		ShaderSetType::QUAD,
		ShaderSetType::GLTF
	};

	using ShaderUseID = uint8_t;

	/* Returns true if @useID equals @other or has it set via bitwise-OR'ing. */
	inline constexpr [[nodiscard]] bool SupportsUseID(ShaderUseID useID, ShaderUseID other) noexcept;
	inline constexpr [[nodiscard]] ShaderType ShaderTypeOfAssigned(AssignedShaderType assignedType) noexcept;
	inline constexpr [[nodiscard]] ShaderSetType SetTypeOfAssigned(AssignedShaderType assignedType) noexcept;

	struct ShaderID
	{
		inline ShaderID(
			uint32_t index,
			ShaderType type,
			AssignedShaderType assignedType,
			ShaderSetType setType
		)
			: Index(index),
			Type(type),
			AssignedType(assignedType),
			SetType(setType)
		{
		}

		ShaderID() = default;
		~ShaderID() = default;

		inline constexpr [[nodiscard]] bool IsValid() const noexcept;
		inline constexpr operator bool() const noexcept { return IsValid(); }
		inline constexpr [[nodiscard]] bool operator==(ShaderID other) const noexcept;

		static constexpr uint32_t S_INVALID_INDEX = ~static_cast<uint32_t>(0);
		uint32_t Index = S_INVALID_INDEX; /* Index into ShaderData list. */
		ShaderType Type = ShaderType::INVALID;
		AssignedShaderType AssignedType = AssignedShaderType::INVALID;
		ShaderSetType SetType = ShaderSetType::INVALID;
	};

	struct ShaderData
	{
		inline ShaderData(ShaderID id, ComPtr<ID3DBlob> pBytecode, const std::filesystem::path& fileName) noexcept
			: ID(id),
			pBytecode(pBytecode),
			Filename(fileName)
		{
		}

		ShaderData() = default;
		~ShaderData() = default;

		ShaderID ID;
		ComPtr<ID3DBlob> pBytecode;
		std::filesystem::path Filename; /* (no extension) */
	};

#pragma region Shader Sets
	class ShaderRegistry;

	enum class CM_ENGINE_API ShaderUseType : ShaderUseID
	{
		VERTEX = 0b00000001 << 7,
		PIXEL = 0b00000001 << 6,
		COMPUTE = 0b00000001 << 5
	};

	class IShaderSet
	{
	public:
		IShaderSet(ShaderUseID useFlags) noexcept;
		virtual ~IShaderSet() = default;

		IShaderSet(const IShaderSet&) = delete;
		IShaderSet(IShaderSet&&) = delete;
		IShaderSet& operator=(const IShaderSet&) = delete;
		IShaderSet& operator=(IShaderSet&&) = delete;
	public:
		inline static constexpr [[nodiscard]] bool SupportsShaderUse(ShaderSetType setType, ShaderUseID useID) noexcept;
		inline static constexpr [[nodiscard]] bool IsUseIDOf(ShaderSetType setType, ShaderUseID useID) noexcept;

		inline [[nodiscard]] bool UsesVertexShader() const noexcept { return m_UseID & (ShaderUseID)ShaderUseType::VERTEX; }
		inline [[nodiscard]] bool UsesPixelShader() const noexcept { return m_UseID & (ShaderUseID)ShaderUseType::PIXEL; }
		inline [[nodiscard]] bool UsesComputeShader() const noexcept { return m_UseID & (ShaderUseID)ShaderUseType::COMPUTE; }

		virtual void Create(const ComPtr<ID3D11Device>& pDevice, const ShaderRegistry& registry) noexcept = 0;
		virtual void Bind(const ComPtr<ID3D11DeviceContext>& pContext) noexcept = 0;
		virtual [[nodiscard]] ShaderSetType Type() const noexcept = 0;
	protected:
		void CreateVertexShader(
			const ComPtr<ID3D11Device>& pDevice,
			ComPtr<ID3D11VertexShader>& pOutVertexShader,
			const ShaderRegistry& registry,
			ShaderID id
		) noexcept;

		void CreatePixelShader(
			const ComPtr<ID3D11Device>& pDevice,
			ComPtr<ID3D11PixelShader>& pOutPixelShader,
			const ShaderRegistry& registry,
			ShaderID id
		) noexcept;

		void CreateVertexAndPixelShader(
			const ComPtr<ID3D11Device>& pDevice,
			ComPtr<ID3D11VertexShader>& pOutVertexShader,
			ComPtr<ID3D11PixelShader>& pOutPixelShader,
			const ShaderRegistry& registry,
			ShaderID vertexID,
			ShaderID pixelID
		) noexcept;

		void CreateInputLayout(
			const ComPtr<ID3D11Device>& pDevice,
			ComPtr<ID3D11InputLayout>& pOutInputLayout,
			std::span<const D3D11_INPUT_ELEMENT_DESC> inputDescs,
			const ShaderRegistry& registry,
			ShaderID vertexID
		) noexcept;

		void BindVertexShader(const ComPtr<ID3D11DeviceContext>& pContext, const ComPtr<ID3D11VertexShader>& pVertexShader) noexcept;
		void BindPixelShader(const ComPtr<ID3D11DeviceContext>& pContext, const ComPtr<ID3D11PixelShader>& pPixelShader) noexcept;
		void BindInputLayout(const ComPtr<ID3D11DeviceContext>& pContext, const ComPtr<ID3D11InputLayout>& pInputLayout) noexcept;
	protected:
		const ShaderUseID m_UseID;
	};

	class ShaderSetQuad : public IShaderSet
	{
	public:
		inline ShaderSetQuad(ShaderID vertexID, ShaderID pixelID) noexcept
			: IShaderSet(S_USE_ID),
			m_VertexID(vertexID),
			m_PixelID(pixelID)
		{
		}

		virtual void Create(const ComPtr<ID3D11Device>& pDevice, const ShaderRegistry& registry) noexcept override;
		virtual void Bind(const ComPtr<ID3D11DeviceContext>& pContext) noexcept override;
		virtual inline constexpr [[nodiscard]] ShaderSetType Type() const noexcept override { return ShaderSetType::QUAD; };

		inline static constexpr [[nodiscard]] ShaderUseID UseID() noexcept { return S_USE_ID; }
		inline static constexpr [[nodiscard]] bool SupportsUsage(ShaderUseID useID) noexcept { return SupportsUseID(S_USE_ID, useID); }

		inline [[nodiscard]] ShaderID VertexID() const noexcept { return m_VertexID; }
		inline [[nodiscard]] ShaderID PixelID() const noexcept { return m_PixelID; }
		inline [[nodiscard]] const ComPtr<ID3D11VertexShader>& VertexShader() noexcept { return mP_VertexShader; }
		inline [[nodiscard]] const ComPtr<ID3D11PixelShader>& PixelShader() noexcept { return mP_PixelShader; }
	private:
		static constexpr ShaderUseID S_USE_ID = (ShaderUseID)ShaderUseType::VERTEX | (ShaderUseID)ShaderUseType::PIXEL;
		ShaderID m_VertexID;
		ShaderID m_PixelID;
		ComPtr<ID3D11VertexShader> mP_VertexShader;
		ComPtr<ID3D11PixelShader> mP_PixelShader;
		ComPtr<ID3D11InputLayout> mP_InputLayout;
	};

	class ShaderSetGltf : public IShaderSet
	{
	public:
		inline ShaderSetGltf(ShaderID vertexID, ShaderID pixelID) noexcept
			: IShaderSet(S_USE_ID),
			  m_VertexID(vertexID),
			  m_PixelID(pixelID)
		{
		}

		virtual void Create(const ComPtr<ID3D11Device>& pDevice, const ShaderRegistry& registry) noexcept override;
		virtual void Bind(const ComPtr<ID3D11DeviceContext>& pContext) noexcept override;
		virtual inline constexpr [[nodiscard]] ShaderSetType Type() const noexcept override { return ShaderSetType::GLTF; };

		inline static constexpr [[nodiscard]] ShaderUseID UseID() noexcept { return S_USE_ID; }
		inline static constexpr [[nodiscard]] bool SupportsUsage(ShaderUseID useID) noexcept { return SupportsUseID(S_USE_ID, useID); }

		inline [[nodiscard]] ShaderID VertexID() const noexcept { return m_VertexID; }
		inline [[nodiscard]] ShaderID PixelID() const noexcept { return m_PixelID; }
		inline [[nodiscard]] const ComPtr<ID3D11VertexShader>& VertexShader() noexcept { return mP_VertexShader; }
		inline [[nodiscard]] const ComPtr<ID3D11PixelShader>& PixelShader() noexcept { return mP_PixelShader; }
	private:
		static constexpr ShaderUseID S_USE_ID = (ShaderUseID)ShaderUseType::VERTEX | (ShaderUseID)ShaderUseType::PIXEL;
		ShaderID m_VertexID;
		ShaderID m_PixelID;
		ComPtr<ID3D11VertexShader> mP_VertexShader;
		ComPtr<ID3D11PixelShader> mP_PixelShader;
		ComPtr<ID3D11InputLayout> mP_InputLayout;
	};
#pragma endregion

	inline constexpr [[nodiscard]] bool ShaderID::IsValid() const noexcept
	{
		return Index != S_INVALID_INDEX &&
			Type != ShaderType::INVALID &&
			AssignedType != AssignedShaderType::INVALID &&
			SetType != ShaderSetType::INVALID;
	}

	inline constexpr [[nodiscard]] bool ShaderID::operator==(ShaderID other) const noexcept
	{
		return Index == other.Index &&
			Type == other.Type &&
			AssignedType == other.AssignedType &&
			SetType == other.SetType;
	}

	inline constexpr [[nodiscard]] bool IShaderSet::SupportsShaderUse(ShaderSetType setType, ShaderUseID useID) noexcept
	{
		switch (setType)
		{
		case ShaderSetType::INVALID: [[fallthrough]];
		case ShaderSetType::TOTAL_SETS: [[fallthrough]];
		default:
			return false;
		case ShaderSetType::GLTF:
			return ShaderSetGltf::SupportsUsage(useID);
		case ShaderSetType::QUAD:
			return ShaderSetQuad::SupportsUsage(useID);
		}
	}

	inline constexpr [[nodiscard]] bool IShaderSet::IsUseIDOf(ShaderSetType setType, ShaderUseID useID) noexcept
	{
		switch (setType)
		{
		case ShaderSetType::INVALID: [[fallthrough]];
		case ShaderSetType::TOTAL_SETS: [[fallthrough]];
		default:
			return false;
		case ShaderSetType::GLTF:
			return ShaderSetGltf::UseID() == useID;
		case ShaderSetType::QUAD:
			return ShaderSetQuad::UseID() == useID;
		}
	}

	inline constexpr [[nodiscard]] bool SupportsUseID(ShaderUseID useID, ShaderUseID other) noexcept
	{
		return useID == other || (useID & other);
	}

	inline constexpr [[nodiscard]] ShaderType ShaderTypeOfAssigned(AssignedShaderType assignedType) noexcept
	{
		switch (assignedType)
		{
		case AssignedShaderType::INVALID: [[fallthrough]];
		case AssignedShaderType::CUSTOM: [[fallthrough]];
		default:
			return ShaderType::INVALID;
		case AssignedShaderType::GLTF_VS: [[fallthrough]];
		case AssignedShaderType::QUAD_VS:
			return ShaderType::VERTEX;
		case AssignedShaderType::GLTF_PS: [[fallthrough]];
		case AssignedShaderType::QUAD_PS:
			return ShaderType::PIXEL;
		}
	}

	inline constexpr [[nodiscard]] ShaderSetType SetTypeOfAssigned(AssignedShaderType assignedType) noexcept
	{
		switch (assignedType)
		{
		case AssignedShaderType::INVALID: [[fallthrough]];
		case AssignedShaderType::CUSTOM: [[fallthrough]];
		default:
			return ShaderSetType::INVALID;
		case AssignedShaderType::GLTF_PS: [[fallthrough]];
		case AssignedShaderType::GLTF_VS:
			return ShaderSetType::GLTF;
		case AssignedShaderType::QUAD_PS: [[fallthrough]];
		case AssignedShaderType::QUAD_VS:
			return ShaderSetType::QUAD;
		}
	}
}