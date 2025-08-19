#pragma once

#include <SDL3/SDL.h>
#include <SDLx_gpu/SDL_gpu.h>

#include <string>

#include "Camera.hpp"

namespace Crobots
{

class Renderer
{
public:
    bool Init();
    void Quit();
    void Present();
    void Draw(const std::string& path, float x, float y, float z, float yaw);
    /* TODO: take an Engine to render */

private:
    SDL_Window* m_window;
    SDL_GPUDevice* m_device;
    SDLx_GPURenderer* m_renderer;
    SDL_GPUTexture* m_depthTexture;
    SDL_GPUTexture* m_colorTexture;
    Camera m_camera;
};

}