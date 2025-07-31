#pragma once

#include <SDL3/SDL.h>

#include <cstdint>

namespace Crobots
{

class Window;

class Renderer
{
public:
    Renderer();
    bool Create(Window& window);
    void Destroy(Window& window);
    void Present(Window& window);

private:
    bool CreateDevice(Window& window);
    bool CreatePipelines(Window& window);
    bool ResizeTextures(uint32_t width, uint32_t height);

    enum Texture
    {
        TextureDepth,
        TextureCount,
    };

    enum GraphicsPipeline
    {
        GraphicsPipelineModelVoxObj,
        GraphicsPipelineCount,
    };

    SDL_GPUDevice* m_device;
    SDL_GPUGraphicsPipeline* m_graphicsPipelines[GraphicsPipelineCount];
    SDL_GPUTexture* m_textures[TextureCount];
    SDL_GPUCommandBuffer* m_commandBuffer;
    uint32_t m_width;
    uint32_t m_height;
};

}