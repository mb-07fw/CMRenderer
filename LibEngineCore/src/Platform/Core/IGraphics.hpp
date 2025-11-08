#pragma once

#include "Platform/Core/IUploadable.hpp"
#include "Platform/Core/InputLayout.hpp"
#include "Platform/Core/Shader.hpp"
#include "Types.hpp"

#include <cstdint>
#include <functional>
#include <memory>
#include <span>

namespace CMEngine
{
	template <typename ResourceTy>
	using Resource = std::unique_ptr<ResourceTy>;

	class IGraphics
	{
	public:
		IGraphics() = default;
		virtual ~IGraphics() = default;
	public:
		virtual void Clear(const Color4& color) noexcept = 0;
		virtual void Present() noexcept = 0;

		virtual void Draw(uint32_t numVertices, uint32_t startVertexLocation) noexcept = 0;
		virtual void DrawIndexedInstanced(
			uint32_t indicesPerInstance,
			uint32_t totalInstances,
			uint32_t startIndexLocation,
			int32_t baseVertexLocation,
			uint32_t startInstanceLocation
		) noexcept = 0;

		virtual [[nodiscard]] Resource<IInputLayout> CreateInputLayout(ShaderID vertexID, std::span<const InputElement> elements) noexcept = 0;
		virtual void BindInputLayout(const Resource<IInputLayout>& pInputLayout) noexcept = 0;

		virtual [[nodiscard]] Resource<IBuffer> CreateBuffer(GPUBufferType type, GPUBufferFlag flags = GPUBufferFlag::Default) noexcept = 0;
		virtual void SetBuffer(const Resource<IBuffer>& pBuffer, const void* pData, size_t numBytes) noexcept = 0;
		virtual void BindVertexBuffer(const Resource<IBuffer>& pBuffer, uint32_t strideBytes, uint32_t offsetBytes, uint32_t slot) noexcept = 0;
		virtual void BindIndexBuffer(const Resource<IBuffer>& pBuffer, DataFormat indexFormat, uint32_t startIndex) noexcept = 0;
		virtual void BindConstantBufferVS(const Resource<IBuffer>& pBuffer, uint32_t slot) noexcept = 0;
		virtual void BindConstantBufferPS(const Resource<IBuffer>& pBuffer, uint32_t slot) noexcept = 0;
	};
}