#pragma once

#include <SDL3/SDL.h>

#include <cstdint>
#include <optional>
#include <span>

namespace Crobots
{

class Mesh
{
public:
    static std::optional<Mesh> CreateMesh(SDL_GPUDevice* device, SDL_GPUCopyPass* copyPass, const std::span<float>& vertices, const std::span<uint16_t>& indices);
    static std::optional<Mesh> CreateCubeMesh(SDL_GPUDevice* device, SDL_GPUCopyPass* copyPass);
    static std::optional<Mesh> CreateCubeWireframeMesh(SDL_GPUDevice* device, SDL_GPUCopyPass* copyPass);
    void Destroy(SDL_GPUDevice* device);
    SDL_GPUBuffer* GetVertexBuffer() const;
    SDL_GPUBuffer* GetIndexBuffer() const;
    uint16_t GetIndexCount() const;
    static SDL_GPUIndexElementSize GetIndexElementSize();

private:
    SDL_GPUBuffer* m_vertexBuffer;
    SDL_GPUBuffer* m_indexBuffer;
    uint16_t m_indexCount;
};

}