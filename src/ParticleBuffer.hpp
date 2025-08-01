#pragma once

#include <SDL3/SDL.h>

#include <cstdint>

namespace Crobots
{

class ParticleBuffer
{
public:
    ParticleBuffer();
    bool Create(SDL_GPUDevice* device, SDL_GPUCopyPass* copyPass, uint32_t stride, uint32_t indexCount);
    void Destroy(SDL_GPUDevice* device);
    void Upload(SDL_GPUDevice* device, void* data, uint32_t size);
    void PreUpdate(SDL_GPUDevice* device, SDL_GPUCopyPass* copyPass);
    void PostUpdate(SDL_GPUDevice* device, SDL_GPUCopyPass* copyPass);

private:
    enum BufferType
    {
        BufferRead,
        BufferWrite,
        BufferCount,
    };

    SDL_GPUBuffer* m_buffers[BufferCount];
    SDL_GPUBuffer* m_particleBuffer;
    SDL_GPUBuffer* m_indirectBuffer;
    SDL_GPUTransferBuffer* m_particleTransferBuffer;
    SDL_GPUTransferBuffer* m_downloadTransferBuffer;
    SDL_GPUFence* m_fence;
    uint32_t m_particleBufferSize;
    uint32_t m_particleBufferCapacity;
    uint32_t m_particleTransferBufferCapacity;
    uint8_t* m_particleTransferBufferData;
    uint32_t m_bufferCapacity;
    uint32_t m_stride;
};

}