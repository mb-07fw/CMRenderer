#pragma once

#include "Platform/Core/IUploadable.hpp"
#include "Platform/Core/ITexture.hpp"
#include "Platform/Core/InputLayout.hpp"
#include "Platform/Core/Shader.hpp"
#include "Types.hpp"

#include <cstdint>
#include <functional>
#include <memory>
#include <span>

namespace CMEngine
{
	class IGraphics
	{
	public:
		IGraphics() = default;
		virtual ~IGraphics() = default;
	public:
		virtual void Clear(
			const Color4& color
		) noexcept = 0;

		virtual void Present() noexcept = 0;

		virtual void Draw(
			uint32_t numVertices,
			uint32_t startVertexLocation
		) noexcept = 0;

		virtual void DrawIndexed(
			uint32_t numIndices,
			uint32_t startIndexLocation,
			int32_t baseVertexLocation
		) noexcept = 0;

		virtual void DrawIndexedInstanced(
			uint32_t indicesPerInstance,
			uint32_t totalInstances,
			uint32_t startIndexLocation,
			int32_t baseVertexLocation,
			uint32_t startInstanceLocation
		) noexcept = 0;

		virtual [[nodiscard]] Resource<IInputLayout> CreateInputLayout(
			std::span<const InputElement> elems,
			ShaderID vertexID
		) noexcept = 0;

		virtual void BindInputLayout(
			const Resource<IInputLayout>& inputLayout
		) noexcept = 0;

		virtual [[nodiscard]] Resource<ITexture> CreateTexture(
			std::span<std::byte> data
		) noexcept = 0;

		virtual void BindTexture(
			const Resource<ITexture>& texture
		) noexcept = 0;

		virtual [[nodiscard]] Resource<IBuffer> CreateBuffer(
			GPUBufferType type,
			GPUBufferFlag flags = GPUBufferFlag::Default
		) noexcept = 0;

		virtual void SetBuffer(
			const Resource<IBuffer>& buffer,
			const void* pData,
			size_t numBytes
		) noexcept = 0;

		virtual void BindVertexBuffer(
			const Resource<IBuffer>& buffer, 
			uint32_t strideBytes, 
			uint32_t offsetBytes, 
			uint32_t slot
		) noexcept = 0;

		virtual void BindIndexBuffer(
			const Resource<IBuffer>& buffer, 
			DataFormat indexFormat, 
			uint32_t startIndex
		) noexcept = 0;

		virtual void BindConstantBufferVS(
			const Resource<IBuffer>& buffer,
			uint32_t slot
		) noexcept = 0;

		virtual void BindConstantBufferPS(
			const Resource<IBuffer>& buffer,
			uint32_t slot
		) noexcept = 0;
	};
}