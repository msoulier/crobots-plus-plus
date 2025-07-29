#pragma once

#include <SDL3/SDL.h>

#include <memory>
#include <string_view>

namespace Crobots
{

enum class ModelType
{
    /* A model exported from MagicaVoxel as an OBJ */
    ModelVoxObj,
};

class Model
{
public:
    static std::shared_ptr<Model> Create(SDL_GPUDevice* device, SDL_GPUCopyPass* copyPass, const std::string_view& name);
    virtual bool Load(SDL_GPUDevice* device, SDL_GPUCopyPass* copyPass, const std::string_view& name) = 0;
    virtual void Destroy(SDL_GPUDevice* device) = 0;
    virtual ModelType GetType() const = 0;
};

}