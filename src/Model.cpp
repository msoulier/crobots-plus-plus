#include <SDL3/SDL.h>

#include <memory>
#include <string_view>

#include "Assert.hpp"
#include "Log.hpp"
#include "Model.hpp"
#include "ModelVoxObj.hpp"

namespace Crobots
{

std::shared_ptr<Model> Model::Create(SDL_GPUDevice* device, SDL_GPUCopyPass* copyPass, const std::string_view& name)
{
    std::unique_ptr<Model> model = std::make_unique<ModelVoxObj>();
    CROBOTS_ASSERT(model);
    if (!model->Load(device, copyPass, name))
    {
        CROBOTS_LOG("Failed to load model: %s", name.data());
        return nullptr;
    }
    return model;
}

}