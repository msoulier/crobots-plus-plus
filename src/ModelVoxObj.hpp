#pragma once

#include <SDL3/SDL.h>

#include <memory>
#include <string_view>

#include "Model.hpp"

namespace Crobots
{

class ModelVoxObj : public Model
{
public:
    bool Load(SDL_GPUDevice* device, SDL_GPUCopyPass* copyPass, const std::string_view& name) override;
    void Destroy(SDL_GPUDevice* device) override;
    ModelType GetType() const override { return ModelType::VoxObj; }

private:
    SDL_GPUBuffer* m_vertexBuffer;
    SDL_GPUBuffer* m_indexBuffer;
    SDL_GPUTexture* m_paletteTexture;
    uint16_t m_indexCount;
};

}