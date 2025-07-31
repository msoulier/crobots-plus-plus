#pragma once

#include <SDL3/SDL.h>

#include <cstdint>
#include <string>
#include <unordered_map>

#include "Camera.hpp"

namespace Crobots
{

class Model;
class ModelVoxObj;
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
    bool CreateSamplers(Window& window);
    bool ResizeTextures(uint32_t width, uint32_t height);
    void RenderModels(SDL_GPUTexture* colorTexture);
    void RenderModelVoxObj(SDL_GPURenderPass* renderPass, const std::shared_ptr<ModelVoxObj>& model);

    enum Texture
    {
        TextureDepth,
        TextureCount,
    };

    enum Sampler
    {
        SamplerNearest,
        SamplerCount,
    };

    enum GraphicsPipeline
    {
        GraphicsPipelineModelVoxObj,
        GraphicsPipelineCount,
    };

    SDL_GPUDevice* m_device;
    SDL_GPUGraphicsPipeline* m_graphicsPipelines[GraphicsPipelineCount];
    SDL_GPUTexture* m_textures[TextureCount];
    SDL_GPUSampler* m_samplers[SamplerCount];
    SDL_GPUCommandBuffer* m_commandBuffer;
    uint32_t m_width;
    uint32_t m_height;
    Camera m_camera;
    std::unordered_map<std::string, std::shared_ptr<Model>> m_models;
};

}