#pragma once

#include "Backend/Win32/Resources_Win32.hpp"
#include "Backend/RefCounted.hpp"
#include "IApiResources.hpp"
#include "ImplRefCounted.hpp"
#include "PlatformFailure.hpp"
#include "Common/Cast.hpp"

#include <cstdint>
#include <span>
#include <type_traits>
#include <vector>

#include <d3dcommon.h>

namespace Platform::Backend::Win32::D3D::_11
{
#pragma region Shader Stuff
    inline constexpr [[nodiscard]] bool IsValidType(ShaderType type) noexcept
    {
        switch (type)
        {
        case ShaderType::Vertex: return true;
        case ShaderType::Pixel:  return true;
        case ShaderType::Invalid: [[fallthrough]];
        default:
            return false;
        }
    }

    template <typename Base, typename D3D11Interface>
        requires std::is_base_of_v<IShader, Base>
    struct Shader final : public Base, public RefCounted
    {
    private:
        using Self = Shader<Base, D3D11Interface>;
    public:
        Shader() = default;
        ~Shader() = default;

        inline void Create(ComPtr<::ID3D11Device>& pDevice) noexcept;
        inline void Bind(ComPtr<::ID3D11DeviceContext>& pContext) noexcept;

        inline ComPtr<::ID3DBlob>& Impl_GetBytecode() noexcept { return mP_Bytecode; }
        inline const ComPtr<::ID3DBlob>& Impl_GetBytecode() const noexcept { return mP_Bytecode; }

        impl_ref_count;
    private:
        ComPtr<D3D11Interface> mP_Shader;
        ComPtr<::ID3DBlob> mP_Bytecode;
    };

    template <typename Base, typename D3D11Interface>
        requires std::is_base_of_v<IShader, Base>
    inline void Shader<Base, D3D11Interface>::Create(ComPtr<::ID3D11Device>& pDevice) noexcept
    {
        PLATFORM_FAILURE_IF(!pDevice, "(Shader) Provided device is nullptr.");
        PLATFORM_FAILURE_IF(!mP_Bytecode, "(Shader) Bytecode is nullptr.");

        ::HRESULT hr = S_OK;

        if constexpr (std::is_same_v<D3D11Interface, ID3D11VertexShader>)
            hr = pDevice->CreateVertexShader(
                mP_Bytecode->GetBufferPointer(),
                mP_Bytecode->GetBufferSize(),
                nullptr,
                &mP_Shader
            );
        else if (std::is_same_v<D3D11Interface, ID3D11PixelShader>)
            hr = pDevice->CreatePixelShader(
                mP_Bytecode->GetBufferPointer(),
                mP_Bytecode->GetBufferSize(),
                nullptr,
                &mP_Shader
            );
        else
            PLATFORM_FAILURE("(Shader_D3D11) Unsupported D3D11Interface type.");

        PLATFORM_FAILURE_IF_V(
            FAILED(hr),
            "(Shader_D3D11) Failed to create shader. Error: {}",
            hr
        );
    }

    template <typename Base, typename D3D11Interface>
        requires std::is_base_of_v<IShader, Base>
    inline void Shader<Base, D3D11Interface>::Bind(ComPtr<::ID3D11DeviceContext>& pContext) noexcept
    {
        PLATFORM_FAILURE_IF(!pContext, "(Shader_D3D11) Provided context is nullptr.");
        PLATFORM_FAILURE_IF(!mP_Shader, "(Shader_D3D11) Shader interface is nullptr.");

        ::HRESULT hr = S_OK;

        if constexpr (std::is_same_v<D3D11Interface, ID3D11VertexShader>)
            pContext->VSSetShader(mP_Shader.Get(), nullptr, 0);
        else if (std::is_same_v<D3D11Interface, ID3D11PixelShader>)
            pContext->PSSetShader(mP_Shader.Get(), nullptr, 0);
        else
            PLATFORM_FAILURE_IF(false, "(Shader_D3D11) Unsupported D3D11Interface type.");
    }

    using VertexShader = Shader<IVertexShader, ::ID3D11VertexShader>;
    using PixelShader = Shader<IPixelShader, ::ID3D11PixelShader>;

#pragma endregion

    inline constexpr [[nodiscard]] size_t BytesOfDataFormat(::DXGI_FORMAT format) noexcept;
    inline constexpr [[nodiscard]] ::DXGI_FORMAT DataFormatToDXGI(DataFormat format) noexcept;
    inline constexpr [[nodiscard]] ::D3D11_INPUT_CLASSIFICATION InputClassToD3D11(InputClass inputClass) noexcept;

    struct InputLayout final : public IInputLayout, public RefCounted
    {
        InputLayout() = default;
        ~InputLayout() = default;

        void Create(
            const ComPtr<::ID3DBlob>& pVSBytecodeWithInputSignature,
            const ComPtr<::ID3D11Device>& pDevice,
            std::span<const InputElement> elems
        ) noexcept;

        void Bind(const ComPtr<::ID3D11DeviceContext>& pContext) const noexcept;

        impl_ref_count;

        virtual [[nodiscard]] const InputElement* Elements() const noexcept override { return m_Elems.data(); }
        virtual [[nodiscard]] size_t NumElements() const noexcept override { return m_Elems.size(); }
    private:
        void TranslateElements(
            std::span<const InputElement> elems,
            std::vector<::D3D11_INPUT_ELEMENT_DESC>& outDescs
        ) noexcept;
    private:
        std::vector<InputElement> m_Elems;
        /* Note: Since D3D11_INPUT_ELEMENT_DESC's hold c-strings, 
         *         they will hold invalid pointers when their respective
         *         Platform::String objects in m_Elems get destroyed. */
        std::vector<::D3D11_INPUT_ELEMENT_DESC> m_Descs;
        ComPtr<::ID3D11InputLayout> mP_InputLayout;
    };

    enum class CBBindStage : uint8_t
    {
        Invalid,
        Vertex,
        Pixel
    };

    inline constexpr [[nodiscard]] ::D3D11_BIND_FLAG BufferTypeToD3D11BindFlags(BufferType type) noexcept;
    inline constexpr [[nodiscard]] ::D3D11_USAGE BufferPolicyToD3D11Usage(BufferPolicy policy) noexcept;
    inline constexpr [[nodiscard]] ::UINT BufferPolicyToD3D11CPUAccessFlags(BufferPolicy policy) noexcept;
    
    template <typename Base, BufferType Type>
        requires std::is_base_of_v<IBuffer, Base>
    struct Buffer : public Base, public RefCounted
    {
        inline Buffer(BufferPolicy policy) noexcept;
        ~Buffer() = default;

        inline void Set(
            const ComPtr<::ID3D11Device>& pDevice,
            const ComPtr<::ID3D11DeviceContext>& pContext,
            const std::span<const Byte>& data
        ) noexcept;

        inline [[nodiscard]] bool IsCreated() const noexcept { return mP_Buffer.Get() != nullptr; }
    private:
        inline void Create(
            const ComPtr<::ID3D11Device>& pDevice,
            const std::span<const Byte>& data
        ) noexcept;

        inline void Update(
            const ComPtr<::ID3D11DeviceContext>& pContext,
            const std::span<const Byte>& data
        ) noexcept;

        inline void EnforceValidPolicy() noexcept;
    public:
        impl_ref_count;
    protected:
        static constexpr BufferType S_Type = Type;
        BufferPolicy m_Policy = BufferPolicy::Default;
        ::D3D11_BUFFER_DESC m_Desc = {};
        ComPtr<::ID3D11Buffer> mP_Buffer;
    };

    struct VertexBuffer final : public Buffer<IVertexBuffer, BufferType::Vertex>
    {
        using Parent = Buffer<IVertexBuffer, BufferType::Vertex>;

        VertexBuffer(BufferPolicy policy) noexcept;
        ~VertexBuffer() = default;

        void Set(
            const ComPtr<::ID3D11Device>& pDevice,
            const ComPtr<::ID3D11DeviceContext>& pContext,
            const std::span<const Byte>& data,
            uint32_t strideBytes,
            uint32_t offsetBytes,
            uint32_t registerSlot
        ) noexcept;

        void Bind(const ComPtr<::ID3D11DeviceContext>& pContext) noexcept;
    private:
        static constexpr ::UINT S_NumRegisterSlots = D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT;
        ::UINT m_StrideBytes = 0;
        ::UINT m_OffsetBytes = 0;
        ::UINT m_RegisterSlot = 0;
    };

    struct IndexBuffer final : public Buffer<IIndexBuffer, BufferType::Index>
    {
        using Parent = Buffer<IIndexBuffer, BufferType::Index>;

        IndexBuffer(BufferPolicy policy) noexcept;
        ~IndexBuffer() = default;

        void Bind(const ComPtr<::ID3D11DeviceContext>& pContext) noexcept;
    private:
        ::DXGI_FORMAT m_IndexFormat = ::DXGI_FORMAT_UNKNOWN;
        ::UINT m_IndexStartOffset = 0;
    };

    struct ConstantBuffer final : public Buffer<IConstantBuffer, BufferType::Constant>
    {
        using Parent = Buffer<IConstantBuffer, BufferType::Constant>;

        ConstantBuffer(BufferPolicy policy) noexcept;
        ~ConstantBuffer() = default;

        void Bind(const ComPtr<::ID3D11DeviceContext>& pContext) noexcept;

    private:
        static constexpr ::UINT S_TotalRegisters = D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT;
        /* Specifies which pipeline stage the constant buffer should be bound to. */
        CBBindStage m_BindStage = CBBindStage::Invalid;
        ::UINT m_RegisterSlot = 0;
    };

    template <typename Base, BufferType Type>
        requires std::is_base_of_v<IBuffer, Base>
    inline Buffer<Base, Type>::Buffer(BufferPolicy policy) noexcept
        : m_Policy(policy)
    {
        EnforceValidPolicy();

        m_Desc.Usage = BufferPolicyToD3D11Usage(m_Policy);
        m_Desc.BindFlags = BufferTypeToD3D11BindFlags(Type);
        m_Desc.CPUAccessFlags = BufferPolicyToD3D11CPUAccessFlags(m_Policy);
    }

    template <typename Base, BufferType Type>
        requires std::is_base_of_v<IBuffer, Base>
    inline void Buffer<Base, Type>::Set(
        const ComPtr<::ID3D11Device>& pDevice,
        const ComPtr<::ID3D11DeviceContext>& pContext,
        const std::span<const Byte>& data
    ) noexcept
    {
        PLATFORM_FAILURE_IF(
            data.data() == nullptr,
            "(Buffer<Base, Type>) Provided data is nullptr."
        );

        PLATFORM_FAILURE_IF(
            data.size_bytes() == 0,
            "(Buffer<Base, Type>) Provided data is empty."
        );

        bool isUpdatable = HasPolicy(m_Policy, BufferPolicy::Dynamic_Write);
        bool requiresResize = data.size_bytes() > m_Desc.ByteWidth;

        if (!IsCreated() || !isUpdatable || requiresResize)
            Create(pDevice, data);
        else
            Update(pContext, data);
    }

    template <typename Base, BufferType Type>
        requires std::is_base_of_v<IBuffer, Base>
    inline void Buffer<Base, Type>::Create(
        const ComPtr<::ID3D11Device>& pDevice,
        const std::span<const Byte>& data
    ) noexcept
    {
        PLATFORM_FAILURE_IF(
            pDevice == nullptr,
            "(Buffer<Base, Type>) Provided device is nullptr."
        );

        m_Desc.ByteWidth = Cast<::UINT>(data.size_bytes());
        
        ::D3D11_SUBRESOURCE_DATA subResData = {};
        subResData.pSysMem = data.data();

        ::HRESULT hr = pDevice->CreateBuffer(
            &m_Desc,
            &subResData,
            // this overload for ComPtr (Microsoft::WRL::ComPtr) releases the interface.
            &mP_Buffer
        );

        PLATFORM_FAILURE_IF_V(
            FAILED(hr),
            "(Buffer<Base, Type>) Failed to create buffer. Error: `{}`",
            hr
        );
    }

    template <typename Base, BufferType Type>
        requires std::is_base_of_v<IBuffer, Base>
    inline void Buffer<Base, Type>::Update(
        const ComPtr<::ID3D11DeviceContext>& pContext,
        const std::span<const Byte>& data
    ) noexcept
    {
        PLATFORM_FAILURE_IF(
            pContext == nullptr,
            "(Buffer<Base, Type>) Provided context is nullptr."
        );

        if (HasPolicy(m_Policy, BufferPolicy::Dynamic_Write))
        {
            ::D3D11_MAPPED_SUBRESOURCE mappedResource = {};

            ::HRESULT hr = pContext->Map(
                mP_Buffer.Get(),
                0,
                D3D11_MAP_WRITE_DISCARD,
                0,
                &mappedResource
            );

            PLATFORM_FAILURE_IF_V(
                FAILED(hr),
                "(Buffer<Base, Type>) Failed to map resource for writing. Error: `{}`",
                hr
            );

            /* Copy data into resource.
             * Note: It is expected that data.size_bytes() is <= current byte width. */
            std::memcpy(mappedResource.pData, data.data(), data.size_bytes());

            pContext->Unmap(mP_Buffer.Get(), 0);
            return;
        }
       
        pContext->UpdateSubresource(
            mP_Buffer.Get(),
            0,
            nullptr,
            data.data(),
            data.size_bytes(),
            0
        );
    }

    template <typename Base, BufferType Type>
        requires std::is_base_of_v<IBuffer, Base>
    inline void Buffer<Base, Type>::EnforceValidPolicy() noexcept
    {
        /* If the basic no-read-write policies are set, there's no need to continue. */
        if (m_Policy == BufferPolicy::Default ||
            m_Policy == BufferPolicy::Immutable)
            return;

        bool containsBasicUsage = false;
        bool containsMultipleBasicUsage = false;

        if (HasPolicy(m_Policy, BufferPolicy::Default))
        {
            containsBasicUsage = true;

            PLATFORM_FAILURE_IF(
                HasPolicy(m_Policy, BufferPolicy::Read) ||
                HasPolicy(m_Policy, BufferPolicy::Write),
                "(Buffer<Base, Type>) Provided 'Default' BufferPolicy contains Read/Write policies. This is invalid "
                "since a 'Default' Buffer isn't permitted any direct CPU access for the best GPU-side memory "
                "optimization. If only CPU-side writing is required, the Dynamic_Write policy should be used, "
                "otherwise Staging_Read, Staging_Write, or Staging_ReadWrite are required."
            );
        }

        if (HasPolicy(m_Policy, BufferPolicy::Immutable))
        {
            containsMultipleBasicUsage = containsBasicUsage;
            containsBasicUsage = true;

            PLATFORM_FAILURE_IF(
                HasPolicy(m_Policy, BufferPolicy::Read) ||
                HasPolicy(m_Policy, BufferPolicy::Write),
                "(Buffer<Base, Type>) Provided 'Immutable' BufferPolicy contains Read/Write policies. This "
                "is invalid since an 'Immutable' Buffer isn't permitted any direct CPU access for the best GPU-side "
                "memory optimization. If only CPU-side writing is required, the Dynamic_Write policy should be used, "
                "otherwise Staging_Read, Staging_Write, or Staging_ReadWrite are required."
            );
        }

        if (HasPolicy(m_Policy, BufferPolicy::Dynamic))
        {
            containsMultipleBasicUsage = containsBasicUsage;
            containsBasicUsage = true;

            if (HasPolicy(m_Policy, BufferPolicy::Read))
                LogWarning(
                    "(Buffer<Base, Type>) Provided 'Dynamic' BufferPolicy contains Read policy. This is fine, "
                    "as it is possible to read data on the CPU from a 'Dynamic' buffer, but is usually bad practice "
                    "due to 'Dynamic' being optimized for high-frequency CPU-Write operations, and will therefore "
                    "incur an unneccesary performance toll compared to a 'Staging' Buffer, which is more suited to general "
                    "data transfer between CPU and GPU."
                );
            if (!HasPolicy(m_Policy, BufferPolicy::Write))
                m_Policy |= BufferPolicy::Write;
        }

        if (HasPolicy(m_Policy, BufferPolicy::Staging))
        {
            containsMultipleBasicUsage = containsBasicUsage;

            if (!HasPolicy(m_Policy, BufferPolicy::Read) &&
                !HasPolicy(m_Policy, BufferPolicy::Write))
            {
                LogWarning(
                    "(Buffer<Base, Type>) Provided 'Staging' BufferPolicy doesn't "
                    "specify any CPU access. Read access is assumed."
                );

                m_Policy |= BufferPolicy::Read;
            }
        }

        PLATFORM_FAILURE_IF(
            containsMultipleBasicUsage,
            "(Buffer<Base, Type>) Provided BufferPolicy contains multiple basic usage policies (Default, Immutable, etc). "
            "This is invalid since these policies represent different semantic use cases in the graphics pipeline, "
            "and will most likely result in failure of buffer creation through the graphics api."
        );
    }

    inline constexpr [[nodiscard]] size_t BytesOfDataFormat(::DXGI_FORMAT format) noexcept
    {
        switch (format)
        {
        case ::DXGI_FORMAT_R8_UINT:               [[fallthrough]];
        case ::DXGI_FORMAT_R8_SINT:               [[fallthrough]];
        case ::DXGI_FORMAT_R8_TYPELESS:           [[fallthrough]];
        case ::DXGI_FORMAT_R8_SNORM:              [[fallthrough]];
        case ::DXGI_FORMAT_R8_UNORM:			  return sizeof(std::byte);

        case ::DXGI_FORMAT_R8G8_UINT:             [[fallthrough]];
        case ::DXGI_FORMAT_R8G8_SINT:             [[fallthrough]];
        case ::DXGI_FORMAT_R8G8_TYPELESS:         [[fallthrough]];
        case ::DXGI_FORMAT_R8G8_SNORM:            [[fallthrough]];
        case ::DXGI_FORMAT_R8G8_UNORM:			  return sizeof(std::byte) * 2;

        case ::DXGI_FORMAT_R8G8B8A8_UINT:         [[fallthrough]];
        case ::DXGI_FORMAT_R8G8B8A8_SINT:         [[fallthrough]];
        case ::DXGI_FORMAT_R8G8B8A8_TYPELESS:     [[fallthrough]];
        case ::DXGI_FORMAT_R8G8B8A8_SNORM:        [[fallthrough]];
        case ::DXGI_FORMAT_R8G8B8A8_UNORM:		  return sizeof(std::byte) * 4;

        case ::DXGI_FORMAT_R16_UINT:              [[fallthrough]];
        case ::DXGI_FORMAT_R16_SINT:              [[fallthrough]];
        case ::DXGI_FORMAT_R16_FLOAT:             [[fallthrough]];
        case ::DXGI_FORMAT_R16_TYPELESS:          [[fallthrough]];
        case ::DXGI_FORMAT_R16_SNORM:             [[fallthrough]];
        case ::DXGI_FORMAT_R16_UNORM:			  return sizeof(uint16_t);

        case ::DXGI_FORMAT_R16G16_UINT:           [[fallthrough]];
        case ::DXGI_FORMAT_R16G16_SINT:           [[fallthrough]];
        case ::DXGI_FORMAT_R16G16_FLOAT:          [[fallthrough]];
        case ::DXGI_FORMAT_R16G16_TYPELESS:       [[fallthrough]];
        case ::DXGI_FORMAT_R16G16_SNORM:          [[fallthrough]];
        case ::DXGI_FORMAT_R16G16_UNORM:		  return sizeof(uint16_t) * 2;

        case ::DXGI_FORMAT_R16G16B16A16_UINT:     [[fallthrough]];
        case ::DXGI_FORMAT_R16G16B16A16_SINT:     [[fallthrough]];
        case ::DXGI_FORMAT_R16G16B16A16_FLOAT:    [[fallthrough]];
        case ::DXGI_FORMAT_R16G16B16A16_TYPELESS: [[fallthrough]];
        case ::DXGI_FORMAT_R16G16B16A16_SNORM:    [[fallthrough]];
        case ::DXGI_FORMAT_R16G16B16A16_UNORM:	  return sizeof(uint16_t) * 4;

        case ::DXGI_FORMAT_R32_UINT:              [[fallthrough]];
        case ::DXGI_FORMAT_R32_SINT:              [[fallthrough]];
        case ::DXGI_FORMAT_R32_FLOAT:             [[fallthrough]];
        case ::DXGI_FORMAT_R32_TYPELESS:	      return sizeof(uint32_t);

        case ::DXGI_FORMAT_R32G32_UINT:           [[fallthrough]];
        case ::DXGI_FORMAT_R32G32_SINT:           [[fallthrough]];
        case ::DXGI_FORMAT_R32G32_FLOAT:          [[fallthrough]];
        case ::DXGI_FORMAT_R32G32_TYPELESS:       return sizeof(uint32_t) * 2;

        case ::DXGI_FORMAT_R32G32B32_UINT:        [[fallthrough]];
        case ::DXGI_FORMAT_R32G32B32_SINT:        [[fallthrough]];
        case ::DXGI_FORMAT_R32G32B32_FLOAT:       [[fallthrough]];
        case ::DXGI_FORMAT_R32G32B32_TYPELESS:    return sizeof(uint32_t) * 3;

        case ::DXGI_FORMAT_R32G32B32A32_UINT:     [[fallthrough]];
        case ::DXGI_FORMAT_R32G32B32A32_SINT:     [[fallthrough]];
        case ::DXGI_FORMAT_R32G32B32A32_FLOAT:    [[fallthrough]];
        case ::DXGI_FORMAT_R32G32B32A32_TYPELESS: return sizeof(uint32_t) * 4;

        default:								  return 0;
        }
    }

    inline constexpr [[nodiscard]] ::DXGI_FORMAT DataFormatToDXGI(DataFormat format) noexcept
    {
        switch (format)
        {
        case DataFormat::Unspecified:         [[fallthrough]];
        case DataFormat::Count:		          return ::DXGI_FORMAT_UNKNOWN;

        case DataFormat::Int8:		          return ::DXGI_FORMAT_R8_SINT;
        case DataFormat::UInt8:		          return ::DXGI_FORMAT_R8_UINT;

        case DataFormat::Int8x2:	          return ::DXGI_FORMAT_R8G8_SINT;
        case DataFormat::UInt8x2:	          return ::DXGI_FORMAT_R8G8_UINT;

        case DataFormat::Int8x3:              [[fallthrough]];
        case DataFormat::Int8x4:	          return ::DXGI_FORMAT_R8G8B8A8_SINT;

        case DataFormat::UInt8x3:             [[fallthrough]];
        case DataFormat::UInt8x4:	          return ::DXGI_FORMAT_R8G8B8A8_UINT;

        case DataFormat::Int16:		          return ::DXGI_FORMAT_R16_SINT;
        case DataFormat::UInt16:	          return ::DXGI_FORMAT_R16_UINT;

        case DataFormat::Int16x2:	          return ::DXGI_FORMAT_R16G16_SINT;
        case DataFormat::UInt16x2:	          return ::DXGI_FORMAT_R16G16_UINT;

        case DataFormat::Int16x3:             [[fallthrough]];
        case DataFormat::Int16x4:	          return ::DXGI_FORMAT_R16G16B16A16_SINT;

        case DataFormat::UInt16x3:            [[fallthrough]];
        case DataFormat::UInt16x4:	          return ::DXGI_FORMAT_R16G16B16A16_UINT;

        case DataFormat::UInt32:	          return ::DXGI_FORMAT_R32_UINT;
        case DataFormat::UInt32x2:	          return ::DXGI_FORMAT_R32G32_UINT;
        case DataFormat::UInt32x3:            return ::DXGI_FORMAT_R32G32B32_UINT;
        case DataFormat::UInt32x4:            return ::DXGI_FORMAT_R32G32B32A32_UINT;

        case DataFormat::Float32:	          return ::DXGI_FORMAT_R32_FLOAT;
        case DataFormat::Float32x2:           return ::DXGI_FORMAT_R32G32_FLOAT;
        case DataFormat::Float32x3:           return ::DXGI_FORMAT_R32G32B32_FLOAT;
        case DataFormat::Float32x4:           return ::DXGI_FORMAT_R32G32B32A32_FLOAT;

            /* It is up to the caller to know that this is a matrix format, as there is no
                corresponding DXGI_FORMAT. The data format for each row is returned. */
        case DataFormat::Mat4_Float32:	      return ::DXGI_FORMAT_R32G32B32A32_FLOAT;

        default:							  return ::DXGI_FORMAT_UNKNOWN;
        }
    }

    inline constexpr [[nodiscard]] ::D3D11_INPUT_CLASSIFICATION InputClassToD3D11(InputClass inputClass) noexcept
    {
        switch (inputClass)
        {
        case InputClass::Invalid: [[fallthrough]];
        default:
            return Cast<::D3D11_INPUT_CLASSIFICATION>(-1);

        case InputClass::PerVertex:   return ::D3D11_INPUT_PER_VERTEX_DATA;
        case InputClass::PerInstance: return ::D3D11_INPUT_PER_INSTANCE_DATA;
        }
    }

    inline constexpr [[nodiscard]] ::D3D11_BIND_FLAG BufferTypeToD3D11BindFlags(BufferType type) noexcept
    {
        switch (type)
        {
        case BufferType::Invalid:  [[fallthrough]];
        default:                   return Cast<::D3D11_BIND_FLAG>(-1);
        case BufferType::Vertex:   return ::D3D11_BIND_VERTEX_BUFFER;
        case BufferType::Index:    return ::D3D11_BIND_INDEX_BUFFER;
        case BufferType::Constant: return ::D3D11_BIND_CONSTANT_BUFFER;
        }
    }

    inline constexpr [[nodiscard]] ::D3D11_USAGE BufferPolicyToD3D11Usage(BufferPolicy policy) noexcept
    {
        ::UINT usage = 0;

        /* Multiple basic usage flags are likely to result in buffer creation failure,
         *   but it is not this function's responsibility to respond accordingly. */
        if (HasPolicy(policy, BufferPolicy::Default))
            usage |= ::D3D11_USAGE_DEFAULT;
        if (HasPolicy(policy, BufferPolicy::Immutable))
            usage |= ::D3D11_USAGE_IMMUTABLE;
        if (HasPolicy(policy, BufferPolicy::Dynamic))
            usage |= ::D3D11_USAGE_DYNAMIC;
        if (HasPolicy(policy, BufferPolicy::Staging))
            usage |= ::D3D11_USAGE_STAGING;

        return Cast<::D3D11_USAGE>(usage);
    }

    inline constexpr [[nodiscard]] ::UINT BufferPolicyToD3D11CPUAccessFlags(BufferPolicy policy) noexcept
    {
        ::UINT cpuFlags = 0;

        if (HasPolicy(policy, BufferPolicy::Read))
            cpuFlags |= ::D3D11_CPU_ACCESS_READ;
        if (HasPolicy(policy, BufferPolicy::Write))
            cpuFlags |= ::D3D11_CPU_ACCESS_WRITE;

        return cpuFlags;
    }
}

#include "ImplRefCountedUndef.hpp"