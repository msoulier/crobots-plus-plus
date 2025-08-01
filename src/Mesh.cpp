#include <SDL3/SDL.h>

#include "Mesh.hpp"

namespace Crobots
{

Mesh Mesh::CreateCubeMesh(SDL_GPUDevice* device)
{
    /* TODO: */
    return {};
}

void Mesh::Destroy(SDL_GPUDevice* device)
{
    SDL_ReleaseGPUBuffer(device, m_vertexBuffer);
    SDL_ReleaseGPUBuffer(device, m_indexBuffer);
}

SDL_GPUBuffer* Mesh::GetVertexBuffer() const
{
    return m_vertexBuffer;
}

SDL_GPUBuffer* Mesh::GetIndexBuffer() const
{
    return m_indexBuffer;
}

uint16_t Mesh::GetIndexCount() const
{
    return m_indexCount;
}

SDL_GPUIndexElementSize Mesh::GetIndexElementSize()
{
    return SDL_GPU_INDEXELEMENTSIZE_16BIT;
}

}