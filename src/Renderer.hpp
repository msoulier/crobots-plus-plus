#pragma once

#include <SDL3/SDL.h>
#include <SDLx_gpu/SDL_gpu.h>

#include <string>

namespace Crobots
{

class Camera;
class Engine;

class Renderer
{
public:
    bool Init();
    void Quit();
    void Present(const Engine& engine, Camera& canera);
    void Draw(const std::string& path, float x, float y, float z, float yaw);
    void Draw(const std::string& text, float x, float y, Uint32 color);

private:
    SDL_Window* m_window;
    SDL_GPUDevice* m_device;
    SDLx_GPURenderer* m_renderer;
    SDL_GPUTexture* m_depthTexture;
    SDL_GPUTexture* m_colorTexture;
};

}