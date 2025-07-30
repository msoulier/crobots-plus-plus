#pragma once

#include <SDL3/SDL.h>

#include <cstdint>

namespace Crobots
{

class Window;

class Renderer
{
public:
    bool Create(Window& window);
    void Destroy(Window& window);
    void Present(Window& window);

private:
    bool CreateDevice();

private:
    SDL_GPUDevice* m_device;
};

}