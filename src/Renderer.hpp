#pragma once

// FIXME: stop using vsync

#include <SDL3/SDL.h>
#include <SDLx_gpu/SDL_gpu.h>

#include <cstdint>
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

    /**
     * @param path The name of the robot model
     * @param x The x position (center of model)
     * @param y The y position (bottom of model)
     * @param z The z position (center of model)
     * @param yaw The rotation around the y axis (radians)
     * @param scale The scale of the robot (robots use 0.1f)
     */
    void Draw(const std::string& path, float x, float y, float z, float yaw, float scale);
    void Draw(const std::string& text, float x, float y, uint32_t color);

private:
    SDL_Window* m_window;
    SDL_GPUDevice* m_device;
    SDLx_GPURenderer* m_renderer;
    SDL_GPUTexture* m_depthTexture;
    SDL_GPUTexture* m_colorTexture;
};

}
