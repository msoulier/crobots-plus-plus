#include <SDL3/SDL.h>

#include <cstdint>
#include <cstring>
#include <optional>
#include <span>

#include "Log.hpp"
#include "Mesh.hpp"

namespace Crobots
{

std::optional<Mesh> Mesh::CreateMesh(SDL_GPUDevice* device, SDL_GPUCopyPass* copyPass, const std::span<float>& vertices, const std::span<uint16_t>& indices)
{
    SDL_GPUTransferBuffer* vertexTransferBuffer;
    SDL_GPUTransferBuffer* indexTransferBuffer;
    {
        SDL_GPUTransferBufferCreateInfo info{};
        info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        info.size = vertices.size_bytes();
        vertexTransferBuffer = SDL_CreateGPUTransferBuffer(device, &info);
        info.size = indices.size_bytes();
        indexTransferBuffer = SDL_CreateGPUTransferBuffer(device, &info);
        if (!vertexTransferBuffer || !indexTransferBuffer)
        {
            CROBOTS_LOG("Failed to create transfer buffer(s): %s", SDL_GetError());
            return {};
        }
    }
    void* vertexData = SDL_MapGPUTransferBuffer(device, vertexTransferBuffer, false);
    void* indexData = SDL_MapGPUTransferBuffer(device, indexTransferBuffer, false);
    if (!vertexData || !indexData)
    {
        CROBOTS_LOG("Failed to map transfer buffer(s): %s", SDL_GetError());
        return {};
    }
    std::memcpy(vertexData, vertices.data(), vertices.size_bytes());
    std::memcpy(indexData, indices.data(), indices.size_bytes());
    SDL_UnmapGPUTransferBuffer(device, vertexTransferBuffer);
    SDL_UnmapGPUTransferBuffer(device, indexTransferBuffer);
    Mesh mesh;
    {
        SDL_GPUBufferCreateInfo info{};
        info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
        info.size = vertices.size_bytes();
        mesh.m_vertexBuffer = SDL_CreateGPUBuffer(device, &info);
        info.usage = SDL_GPU_BUFFERUSAGE_INDEX;
        info.size = indices.size_bytes();
        mesh.m_indexBuffer = SDL_CreateGPUBuffer(device, &info);
        if (!mesh.m_vertexBuffer || !mesh.m_indexBuffer)
        {
            CROBOTS_LOG("Failed to create buffer(s): %s", SDL_GetError());
            return {};
        }
    }
    SDL_GPUTransferBufferLocation location{};
    SDL_GPUBufferRegion region{};
    location.transfer_buffer = vertexTransferBuffer;
    region.buffer = mesh.m_vertexBuffer;
    region.size = vertices.size_bytes();
    SDL_UploadToGPUBuffer(copyPass, &location, &region, false);
    location.transfer_buffer = indexTransferBuffer;
    region.buffer = mesh.m_indexBuffer;
    region.size = indices.size_bytes();
    SDL_UploadToGPUBuffer(copyPass, &location, &region, false);
    SDL_ReleaseGPUTransferBuffer(device, vertexTransferBuffer);
    SDL_ReleaseGPUTransferBuffer(device, indexTransferBuffer);
    mesh.m_indexCount = indices.size();
    return mesh;
}

std::optional<Mesh> Mesh::CreateCubeMesh(SDL_GPUDevice* device, SDL_GPUCopyPass* copyPass)
{
    static float vertices[24] =
    {
       -0.5f,-0.5f, 0.5f,
        0.5f,-0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
       -0.5f, 0.5f, 0.5f,
       -0.5f,-0.5f,-0.5f,
        0.5f,-0.5f,-0.5f,
        0.5f, 0.5f,-0.5f,
       -0.5f, 0.5f,-0.5f,
    };
    static uint16_t indices[36] =
    {
        0, 1, 2,
        0, 2, 3,
        5, 4, 7,
        5, 7, 6,
        4, 0, 3,
        4, 3, 7,
        1, 5, 6,
        1, 6, 2,
        3, 2, 6,
        3, 6, 7,
        4, 5, 1,
        4, 1, 0,
    };
    return CreateMesh(device, copyPass, {vertices}, {indices});
}

std::optional<Mesh> Mesh::CreateCubeWireframeMesh(SDL_GPUDevice* device, SDL_GPUCopyPass* copyPass)
{
    static float vertices[24] =
    {
       -0.5f,-0.5f, 0.5f,
        0.5f,-0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
       -0.5f, 0.5f, 0.5f,
       -0.5f,-0.5f,-0.5f,
        0.5f,-0.5f,-0.5f,
        0.5f, 0.5f,-0.5f,
       -0.5f, 0.5f,-0.5f,
    };
    static uint16_t indices[24] =
    {
        0, 1,
        1, 2,
        2, 3,
        3, 0,
        4, 5,
        5, 6,
        6, 7,
        7, 4,
        0, 4,
        1, 5,
        2, 6,
        3, 7,
    };
    return CreateMesh(device, copyPass, {vertices}, {indices});
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