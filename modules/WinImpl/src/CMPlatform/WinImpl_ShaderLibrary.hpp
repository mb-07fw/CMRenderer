#pragma once

#include <string_view>
#include <string>
#include <array>
#include <vector>
#include <unordered_map>
#include <memory>
#include <span>
#include <cstdint>

#include <wrl/client.h>
#include <d3dcommon.h>

namespace CMEngine::Platform::WinImpl
{
	enum ShaderType : int8_t
	{
		SHADER_TYPE_INVALID = -1,
		SHADER_TYPE_VERTEX,
		SHADER_TYPE_PIXEL,
		SHADER_TYPE_COMPUTE
	};

	enum AssignedShaderType : int8_t
	{
		ASSIGNED_SHADER_TYPE_INVALID = -1,
		ASSIGNED_SHADER_TYPE_QUAD_VS,
		ASSIGNED_SHADER_TYPE_QUAD_PS,
		ASSIGNED_SHADER_TYPE_CUSTOM
	};

	enum ShaderSetType : int8_t
	{
		SHADER_SET_TYPE_INVALID = -1,
		SHADER_SET_TYPE_QUAD,
		SHADER_SET_TYPE_TOTAL_SETS
	};

	inline constexpr std::array<ShaderSetType, static_cast<size_t>(SHADER_SET_TYPE_TOTAL_SETS)> G_IMPLEMENTED_SHADER_SETS = {
		SHADER_SET_TYPE_QUAD
	};

	inline constexpr [[nodiscard]] bool IsVertexAssignedType(AssignedShaderType assignedType) noexcept;
	inline constexpr [[nodiscard]] ShaderType ShaderTypeOfAssigned(AssignedShaderType assignedType) noexcept;
	inline constexpr [[nodiscard]] ShaderSetType SetTypeOfAssigned(AssignedShaderType assignedType) noexcept;

	template <typename Ty>
	using ComPtr = Microsoft::WRL::ComPtr<Ty>;

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
		ShaderType Type = SHADER_TYPE_INVALID;
		AssignedShaderType AssignedType = ASSIGNED_SHADER_TYPE_INVALID;
		ShaderSetType SetType = SHADER_SET_TYPE_INVALID;
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

	using ShaderUseID = uint8_t;

	enum ShaderUseType : ShaderUseID
	{
		SHADER_USE_VERTEX = 0b00000001 << 7,
		SHADER_USE_PIXEL = 0b00000001 << 6,
		SHADER_USE_COMPUTE = 0b00000001 << 5
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

		inline [[nodiscard]] bool UsesVertexShader() const noexcept { return m_UseID & SHADER_USE_VERTEX; }
		inline [[nodiscard]] bool UsesPixelShader() const noexcept { return m_UseID & SHADER_USE_PIXEL; }
		inline [[nodiscard]] bool UsesComputeShader() const noexcept { return m_UseID & SHADER_USE_COMPUTE; }

		virtual void Create(ComPtr<ID3D11Device> pDevice, const ShaderRegistry& registry) noexcept = 0;
		virtual void Bind(ComPtr<ID3D11DeviceContext> pContext) noexcept = 0;
		virtual [[nodiscard]] ShaderSetType Type() const noexcept = 0;
	protected:
		void CreateVertexShader(
			ComPtr<ID3D11Device> pDevice,
			ComPtr<ID3D11VertexShader>& pOutVertexShader,
			const ShaderRegistry& registry,
			ShaderID id
		) noexcept;

		void CreatePixelShader(
			ComPtr<ID3D11Device> pDevice,
			ComPtr<ID3D11PixelShader>& pOutPixelShader,
			const ShaderRegistry& registry,
			ShaderID id
		) noexcept;

		void CreateVertexAndPixelShader(
			ComPtr<ID3D11Device> pDevice,
			ComPtr<ID3D11VertexShader>& pOutVertexShader,
			ComPtr<ID3D11PixelShader>& pOutPixelShader,
			const ShaderRegistry& registry,
			ShaderID vertexID,
			ShaderID pixelID
		) noexcept;

		void CreateInputLayout(
			ComPtr<ID3D11Device> pDevice,
			ComPtr<ID3D11InputLayout>& pOutInputLayout,
			std::span<const D3D11_INPUT_ELEMENT_DESC> inputDescs,
			const ShaderRegistry& registry,
			ShaderID vertexID
		) noexcept;

		void BindVertexShader(ComPtr<ID3D11DeviceContext> pContext, ComPtr<ID3D11VertexShader> pVertexShader) noexcept;
		void BindPixelShader(ComPtr<ID3D11DeviceContext> pContext, ComPtr<ID3D11PixelShader> pPixelShader) noexcept;
		void BindInputLayout(ComPtr<ID3D11DeviceContext> pContext, ComPtr<ID3D11InputLayout> pInputLayout) noexcept;
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

		void Create(ComPtr<ID3D11Device> pDevice, const ShaderRegistry& registry) noexcept;
		void Bind(ComPtr<ID3D11DeviceContext> pContext) noexcept;

		inline static constexpr [[nodiscard]] ShaderUseID UseID() noexcept { return S_USE_ID; }
		inline static constexpr [[nodiscard]] bool SupportsUsage(ShaderUseID useID) noexcept;

		inline [[nodiscard]] ShaderID VertexID() const noexcept { return m_VertexID; }
		inline [[nodiscard]] ShaderID PixelID() const noexcept { return m_PixelID; }
		inline [[nodiscard]] ComPtr<ID3D11VertexShader> VertexShader() noexcept { return mP_VertexShader; }
		inline [[nodiscard]] ComPtr<ID3D11PixelShader> PixelShader() noexcept { return mP_PixelShader; }

		inline constexpr virtual [[nodiscard]] ShaderSetType Type() const noexcept override { return SHADER_SET_TYPE_QUAD; };
	private:
		static constexpr ShaderUseID S_USE_ID = SHADER_USE_VERTEX | SHADER_USE_PIXEL;
		ShaderID m_VertexID;
		ShaderID m_PixelID;
		ComPtr<ID3D11VertexShader> mP_VertexShader;
		ComPtr<ID3D11PixelShader> mP_PixelShader;
		ComPtr<ID3D11InputLayout> mP_InputLayout;
	};
#pragma endregion

#pragma region Shader Registry
	class ShaderRegistry
	{
	public:
		ShaderRegistry() = default;
		~ShaderRegistry() = default;

		ShaderRegistry(const ShaderRegistry&) = delete;
		ShaderRegistry(ShaderRegistry&&) = delete;
		ShaderRegistry& operator=(const ShaderRegistry&) = delete;
		ShaderRegistry& operator=(ShaderRegistry&&) = delete;
	public:
		[[nodiscard]] ShaderID QueryID(const std::filesystem::path& fileName) const noexcept;

		void Add(ShaderID id, ComPtr<ID3DBlob> pBytecode, const std::filesystem::path& fileName) noexcept;

		[[nodiscard]] const ShaderData* Retrieve(ShaderID id) const noexcept;

		inline [[nodiscard]] const std::vector<ShaderData>& Data() const noexcept { return m_ShaderData; }
	private:
		uint32_t m_NextShaderIndex = 0;
		std::vector<ShaderData> m_ShaderData;
		std::unordered_map<std::wstring, ShaderID> m_ShaderNames = {
			{ { L"Quad_VS" }, { m_NextShaderIndex++, SHADER_TYPE_VERTEX, ASSIGNED_SHADER_TYPE_QUAD_VS, SHADER_SET_TYPE_QUAD } },
			{ { L"Quad_PS" }, { m_NextShaderIndex++, SHADER_TYPE_PIXEL, ASSIGNED_SHADER_TYPE_QUAD_PS, SHADER_SET_TYPE_QUAD } }
		};
	};
#pragma endregion

#pragma region Shader Library
	class ShaderLibrary
	{
	public:
		ShaderLibrary() noexcept;
		~ShaderLibrary() noexcept;
	public:
		void CreateShaderSets(ComPtr<ID3D11Device> pDevice) noexcept;
		void BindSet(ShaderSetType setType, ComPtr<ID3D11DeviceContext> pContext) noexcept;
	private:
		void LoadShaders() noexcept;

		[[nodiscard]] bool ValidateSetSupportsUse(ShaderSetType setType, ShaderType type, const std::filesystem::path& fileName, ShaderUseID useID) noexcept;
	private:
		static constexpr std::string_view S_COMPILED_SHADER_EXT = ".cso";
		static constexpr std::wstring_view S_COMPILED_SHADER_EXTW = L".cso";
		ShaderRegistry m_Registry;
		bool m_CreatedShaderSets = false;
		std::vector<std::shared_ptr<IShaderSet>> m_ShaderSets;
	};
#pragma endregion

	inline constexpr [[nodiscard]] bool IShaderSet::SupportsShaderUse(ShaderSetType setType, ShaderUseID useID) noexcept
	{
		switch (setType)
		{
		case SHADER_SET_TYPE_INVALID: [[fallthrough]];
		case SHADER_SET_TYPE_TOTAL_SETS: [[fallthrough]];
		default:
			return false;

		case SHADER_SET_TYPE_QUAD:
			return ShaderSetQuad::SupportsUsage(useID);
		}
	}

	inline constexpr [[nodiscard]] bool IShaderSet::IsUseIDOf(ShaderSetType setType, ShaderUseID useID) noexcept
	{
		switch (setType)
		{
		case SHADER_SET_TYPE_INVALID: [[fallthrough]];
		case SHADER_SET_TYPE_TOTAL_SETS: [[fallthrough]];
		default:
			return false;

		case SHADER_SET_TYPE_QUAD:
			return ShaderSetQuad::UseID() == useID;
		}
	}

	inline constexpr [[nodiscard]] bool IsVertexAssignedType(AssignedShaderType assignedType) noexcept
	{
		switch (assignedType)
		{
		case ASSIGNED_SHADER_TYPE_INVALID: [[fallthrough]];
		case ASSIGNED_SHADER_TYPE_CUSTOM:  [[fallthrough]];
		case ASSIGNED_SHADER_TYPE_QUAD_PS: [[fallthrough]];
		default:
			return false;
		case ASSIGNED_SHADER_TYPE_QUAD_VS:
			return true;
		}
	}

	inline constexpr [[nodiscard]] ShaderType ShaderTypeOfAssigned(AssignedShaderType assignedType) noexcept
	{
		switch (assignedType)
		{
		case ASSIGNED_SHADER_TYPE_INVALID: [[fallthrough]];
		case ASSIGNED_SHADER_TYPE_CUSTOM: [[fallthrough]];
		default:
			return SHADER_TYPE_INVALID;
		case ASSIGNED_SHADER_TYPE_QUAD_VS:
			return SHADER_TYPE_VERTEX;
		case ASSIGNED_SHADER_TYPE_QUAD_PS:
			return SHADER_TYPE_PIXEL;
		}
	}

	inline constexpr [[nodiscard]] ShaderSetType SetTypeOfAssigned(AssignedShaderType assignedType) noexcept
	{
		switch (assignedType)
		{
		case ASSIGNED_SHADER_TYPE_INVALID: [[fallthrough]];
		case ASSIGNED_SHADER_TYPE_CUSTOM: [[fallthrough]];
		default:
			return SHADER_SET_TYPE_INVALID;
		case ASSIGNED_SHADER_TYPE_QUAD_PS: [[fallthrough]];
		case ASSIGNED_SHADER_TYPE_QUAD_VS:
			return SHADER_SET_TYPE_QUAD;
		}
	}

	inline constexpr [[nodiscard]] bool ShaderID::IsValid() const noexcept
	{
		return Index != S_INVALID_INDEX &&
			Type != SHADER_TYPE_INVALID &&
			AssignedType != ASSIGNED_SHADER_TYPE_INVALID &&
			SetType != SHADER_SET_TYPE_INVALID;
	}

	inline constexpr [[nodiscard]] bool ShaderID::operator==(ShaderID other) const noexcept
	{
		return Index == other.Index &&
			Type == other.Type &&
			AssignedType == other.AssignedType &&
			SetType == other.SetType;
	}

	inline constexpr [[nodiscard]] bool ShaderSetQuad::SupportsUsage(ShaderUseID useID) noexcept
	{
		if (useID == S_USE_ID || (S_USE_ID & useID))
			return true;

		return false;
	}
}