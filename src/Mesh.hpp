#pragma once

#include <SDL3/SDL.h>

namespace Crobots
{

class Mesh
{
public:
    static Mesh CreateCubeMesh(SDL_GPUDevice* device);
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