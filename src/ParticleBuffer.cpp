#include <SDL3/SDL.h>

#include <cstdint>
#include <cstring>

#include "Assert.hpp"
#include "Log.hpp"
#include "ParticleBuffer.hpp"

namespace Crobots
{

ParticleBuffer::ParticleBuffer()
    : m_buffers{}
    , m_particleBuffer{nullptr}
    , m_indirectBuffer{nullptr}
    , m_particleTransferBuffer{nullptr}
    , m_indirectTransferBuffer{nullptr}
    , m_fence{nullptr}
    , m_particleBufferSize{0}
    , m_particleBufferCapacity{0}
    , m_particleTransferBufferCapacity{0}
    , m_particleTransferBufferData{nullptr}
    , m_bufferCapacity{0}
    , m_stride{0} {}

bool ParticleBuffer::Create(SDL_GPUDevice* device, SDL_GPUCopyPass* copyPass, uint32_t stride, uint32_t indexCount)
{
    CROBOTS_ASSERT(stride);
    m_stride = stride;
    {
        SDL_GPUBufferCreateInfo info{};
        info.usage = SDL_GPU_BUFFERUSAGE_INDIRECT;
        info.size = sizeof(SDL_GPUIndexedIndirectDrawCommand);
        m_indirectBuffer = SDL_CreateGPUBuffer(device, &info);
        if (!m_indirectBuffer)
        {
            CROBOTS_LOG("Failed to create buffer: %s", SDL_GetError());
            return false;
        }
    }
    {
        /* only transfering num_instances */
        SDL_GPUTransferBufferCreateInfo info{};
        info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD;
        info.size = sizeof(uint32_t);
        m_indirectTransferBuffer = SDL_CreateGPUTransferBuffer(device, &info);
        if (!m_indirectTransferBuffer)
        {
            CROBOTS_LOG("Failed to create transfer buffer: %s", SDL_GetError());
            return false;
        }
    }
    {
        SDL_GPUTransferBufferCreateInfo info{};
        info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        info.size = sizeof(SDL_GPUIndexedIndirectDrawCommand);
        SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(device, &info);
        if (!transferBuffer)
        {
            CROBOTS_LOG("Failed to create transfer buffer: %s", SDL_GetError());
            return false;
        }
        SDL_GPUIndexedIndirectDrawCommand* data = static_cast<SDL_GPUIndexedIndirectDrawCommand*>(SDL_MapGPUTransferBuffer(device, transferBuffer, false));
        if (!data)
        {
            CROBOTS_LOG("Failed to map transfer buffer: %s", SDL_GetError());
            return false;
        }
        data->num_indices = indexCount;
        data->num_instances = 0;
        data->first_index = 0;
        data->vertex_offset = 0;
        data->first_instance = 0;
        SDL_GPUTransferBufferLocation location{};
        SDL_GPUBufferRegion region{};
        location.transfer_buffer = transferBuffer;
        region.buffer = m_indirectBuffer;
        region.size = sizeof(SDL_GPUIndexedIndirectDrawCommand);
        SDL_UnmapGPUTransferBuffer(device, transferBuffer);
        SDL_UploadToGPUBuffer(copyPass, &location, &region, false);
        SDL_ReleaseGPUTransferBuffer(device, transferBuffer);
    }
    m_bufferCapacity = 10;
    for (int i = 0; i < BufferCount; i++)
    {
        SDL_GPUBufferCreateInfo info{};
        info.usage = SDL_GPU_BUFFERUSAGE_VERTEX |
            SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_READ |
            SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_WRITE;
        info.size = m_bufferCapacity * m_stride;
        m_buffers[i] = SDL_CreateGPUBuffer(device, &info);
        if (!m_buffers[i])
        {
            CROBOTS_LOG("Failed to create buffer: %s", SDL_GetError());
            return false;
        }
    }
    return true;
}

void ParticleBuffer::Destroy(SDL_GPUDevice* device)
{
    for (int i = 0; i < BufferCount; i++)
    {
        SDL_ReleaseGPUBuffer(device, m_buffers[i]);
    }
    SDL_ReleaseGPUBuffer(device, m_particleBuffer);
    SDL_ReleaseGPUBuffer(device, m_indirectBuffer);
    SDL_ReleaseGPUTransferBuffer(device, m_particleTransferBuffer);
    SDL_ReleaseGPUTransferBuffer(device, m_indirectTransferBuffer);
    SDL_ReleaseGPUFence(device, m_fence);
}

void ParticleBuffer::Upload(SDL_GPUDevice* device, void* data, uint32_t size)
{
    if (!size)
    {
        return;
    }
    if (m_particleTransferBuffer && !m_particleTransferBufferData)
    {
        m_particleTransferBufferData = static_cast<uint8_t*>(SDL_MapGPUTransferBuffer(device, m_particleTransferBuffer, true));
        if (!m_particleTransferBufferData)
        {
            CROBOTS_LOG("Failed to map transfer buffer: %s", SDL_GetError());
            return;
        }
    }
    if (m_particleBufferSize + size > m_particleTransferBufferCapacity)
    {
        uint32_t capacity = std::max(10u, (m_particleBufferSize + size) * 2u);
        SDL_GPUTransferBufferCreateInfo info{};
        info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        info.size = capacity * m_stride;
        SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(device, &info);
        if (!transferBuffer)
        {
            CROBOTS_LOG("Failed to create transfer buffer: %s", SDL_GetError());
            return;
        }
        uint8_t* data = static_cast<uint8_t*>(SDL_MapGPUTransferBuffer(device, transferBuffer, true));
        if (!data)
        {
            CROBOTS_LOG("Failed to map transfer buffer: %s", SDL_GetError());
            SDL_ReleaseGPUTransferBuffer(device, transferBuffer);
            return;
        }
        if (m_particleTransferBuffer)
        {
            std::memcpy(data, m_particleTransferBufferData, m_particleBufferSize * m_stride); 
            SDL_UnmapGPUTransferBuffer(device, transferBuffer);
            SDL_UnmapGPUTransferBuffer(device, m_particleTransferBuffer);
            SDL_ReleaseGPUTransferBuffer(device, m_particleTransferBuffer);
        }
        m_particleTransferBuffer = transferBuffer;
        m_particleTransferBufferData = data;
        m_particleTransferBufferCapacity = capacity;
    }
    std::memcpy(m_particleTransferBufferData + m_particleBufferSize, data, size * m_stride);
    m_particleBufferSize += size;
}

void ParticleBuffer::PreUpdate(SDL_GPUDevice* device, SDL_GPUCopyPass* copyPass)
{
    if (m_particleBufferSize > m_particleBufferCapacity)
    {
        SDL_ReleaseGPUBuffer(device, m_particleBuffer);
        m_particleBuffer = nullptr;
        SDL_GPUBufferCreateInfo info{};
        info.usage = SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_READ;
        info.size = m_particleTransferBufferCapacity * m_stride;
        m_particleBuffer = SDL_CreateGPUBuffer(device, &info);
        if (!m_particleBuffer)
        {
            CROBOTS_LOG("Failed to create buffer: %s", SDL_GetError());
            return;
        }
        m_particleBufferCapacity = m_particleTransferBufferCapacity;
    }
    if (m_particleTransferBufferData)
    {
        SDL_UnmapGPUTransferBuffer(device, m_particleTransferBuffer);
        m_particleTransferBufferData = nullptr;
    }
    SDL_GPUTransferBufferLocation location{};
    SDL_GPUBufferRegion region{};
    location.transfer_buffer = m_particleTransferBuffer;
    region.buffer = m_particleBuffer;
    region.size = m_particleBufferSize * m_stride;
    SDL_UploadToGPUBuffer(copyPass, &location, &region, true);
}

void ParticleBuffer::PostUpdate(SDL_GPUDevice* device, SDL_GPUCopyPass* copyPass)
{
    m_particleBufferSize = 0;
    if (SDL_QueryGPUFence(device, m_fence))
    {
        SDL_ReleaseGPUFence(device, m_fence);
        m_fence = nullptr;
        uint32_t* data = static_cast<uint32_t*>(SDL_MapGPUTransferBuffer(device, m_indirectTransferBuffer, false));
        if (!data)
        {
            CROBOTS_LOG("Failed to map transfer buffer: %s", SDL_GetError());
            return;
        }
        uint32_t size = *data;
        SDL_UnmapGPUTransferBuffer(device, m_indirectTransferBuffer);
        CROBOTS_ASSERT(size <= m_particleBufferCapacity);
        if (size == m_particleBufferCapacity)
        {
            uint32_t capacity = std::max(10u, size * 2u);
            for (int i = 0; i < BufferCount; i++)
            {
                SDL_GPUBufferCreateInfo info{};
                info.usage = SDL_GPU_BUFFERUSAGE_VERTEX |
                    SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_READ |
                    SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_WRITE;
                info.size = capacity * m_stride;
                SDL_GPUBuffer* buffer = SDL_CreateGPUBuffer(device, &info);
                if (!buffer)
                {
                    CROBOTS_LOG("Failed to create buffer: %s", SDL_GetError());
                    return;
                }
                if (m_buffers[i])
                {
                    SDL_GPUBufferLocation src{};
                    SDL_GPUBufferLocation dst{};
                    src.buffer = m_buffers[i];
                    dst.buffer = buffer;
                    SDL_CopyGPUBufferToBuffer(copyPass, &src, &dst, size * m_stride, false);
                    SDL_ReleaseGPUBuffer(device, m_buffers[i]);
                }
                m_buffers[i] = buffer;
            }
        }
    }
    if (!m_fence)
    {
        /* submit and get fence */
    }
    else
    {
        /* submit */
    }
}

}