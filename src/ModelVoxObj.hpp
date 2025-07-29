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
    ModelType GetType() const override { return ModelType::ModelVoxObj; }

private:
    SDL_GPUBuffer* vertexBuffer;
    SDL_GPUBuffer* indexBuffer;
    SDL_GPUTexture* paletteTexture;
    uint16_t indexCount;
};

}