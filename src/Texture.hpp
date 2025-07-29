#pragma once

#include <SDL3/SDL.h>

#include <string_view>

namespace Crobots
{

SDL_GPUTexture* LoadTexture(SDL_GPUDevice* device, SDL_GPUCopyPass* copyPass, const std::string_view& path);
SDL_GPUTexture* CreateColorTexture(SDL_GPUDevice* device, SDL_Window* window, int width, int height, SDL_GPUTextureUsageFlags usage = 0);
SDL_GPUTextureFormat GetDepthTextureFormat(SDL_GPUDevice* device);
SDL_GPUTexture* CreateDepthTexture(SDL_GPUDevice* device, int width, int height, SDL_GPUTextureUsageFlags usage = 0);

}