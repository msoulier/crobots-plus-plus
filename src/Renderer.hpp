#pragma once

#include <SDL3/SDL.h>
#include <SDLx_gpu/SDL_gpu.h>
#include <SDLx_model/SDL_model.h>

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>

#include "Camera.hpp"

namespace Crobots
{

class Window;

class Renderer
{
public:
    bool Create(Window& window);
    void Destroy(Window& window);
    void Present(Window& window);
    void Draw(const std::string& model, float x, float y, float z, float yaw);

private:
    void RenderModels(SDL_GPUCommandBuffer* commandBuffer);
    void RenderModelVoxObj(SDL_GPUCommandBuffer* commandBuffer, SDL_GPURenderPass* renderPass, SDLx_Model* model, const glm::mat4& transform);

    struct ModelInstance
    {
        std::string m_model;
        glm::mat4 m_transform;
    };

    SDL_GPUDevice* m_device;
    SDL_GPUGraphicsPipeline* m_modelVoxObjPipeline;
    SDL_GPUTexture* m_depthTexture;
    SDL_GPUTexture* m_colorTexture;
    SDL_GPUSampler* m_nearestSampler;
    std::unordered_map<std::string, SDLx_Model*> m_models;
    std::vector<ModelInstance> m_instances;
    uint32_t m_width;
    uint32_t m_height;
    Camera m_camera;
};

}