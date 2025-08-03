#pragma once

#include <SDL3/SDL.h>

namespace Crobots
{

class Window;

SDL_GPUGraphicsPipeline* CreateModelVoxObjPipeline(SDL_GPUDevice* device, Window& window);
SDL_GPUGraphicsPipeline* CreateParticlePipeline(SDL_GPUDevice* device, Window& window);

}