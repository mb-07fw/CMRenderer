#pragma once

#include "Export.hpp"
#include "IUploadable.hpp"
#include "Types.hpp"

#include <cstdint>
#include <functional>
#include <memory>

namespace CMEngine
{
	struct CM_ENGINE_API DrawDescriptor
	{
		uint32_t TotalVertices = 0;
		uint32_t StartVertexLocation = 0; /* Index of the first vertex to draw. (Offset into the vertex buffer) */
		uint32_t VertexByteStride = 0;
	};

	class CM_ENGINE_API IGraphics
	{
	protected:
		template <typename Ty>
		using BufferPtr = std::unique_ptr<Ty>;
	public:
		IGraphics() = default;
		virtual ~IGraphics() = default;
	public:
		virtual void Clear(const Color4& color) noexcept = 0;
		virtual void Present() noexcept = 0;
		virtual void Draw(const void* pBuffer, const DrawDescriptor& pDescriptor) noexcept = 0;

		virtual void StartFrame(const Color4& clearColor) noexcept = 0;
		virtual void EndFrame() noexcept = 0;

		/* TODO: Worry about ABI stability later. */
		virtual [[nodiscard]] BufferPtr<IUploadable> CreateBuffer(GPUBufferType type, GPUBufferFlag flags = GPUBufferFlag::Default) noexcept = 0;
		virtual void SetBuffer(const BufferPtr<IUploadable>& pBuffer, void* pData, size_t numBytes) noexcept = 0;
		virtual void BindVertexBuffer(const BufferPtr<IUploadable>& pBuffer, UINT strideBytes, UINT offsetBytes, UINT slot) noexcept = 0;
		virtual void BindIndexBuffer(const BufferPtr<IUploadable>& pBuffer, DXGI_FORMAT format, UINT startIndex) noexcept = 0;
		virtual void BindConstantBufferVS(const BufferPtr<IUploadable>& pBuffer, UINT slot) noexcept = 0;
		virtual void BindConstantBufferPS(const BufferPtr<IUploadable>& pBuffer, UINT slot) noexcept = 0;

		virtual [[nodiscard]] bool IsWithinFrame() const noexcept = 0;
	};
}