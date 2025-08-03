#pragma once

#include <SDL3/SDL.h>

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>

#include "Camera.hpp"
#include "Mesh.hpp"
#include "ParticleBuffer.hpp"

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
    bool CreateMeshes(Window& window, SDL_GPUCopyPass* copyPass);
    bool CreateParticleBuffers(Window& window, SDL_GPUCopyPass* copyPass);
    bool ResizeTextures(uint32_t width, uint32_t height);
    void RenderModels(SDL_GPUTexture* colorTexture);
    void RenderModelVoxObj(SDL_GPURenderPass* renderPass, const std::shared_ptr<ModelVoxObj>& model);

    enum TextureType
    {
        TextureDepth,
        TextureCount,
    };

    enum SamplerType
    {
        SamplerNearest,
        SamplerCount,
    };

    enum GraphicsPipelineType
    {
        GraphicsPipelineModelVoxObj,
        GraphicsPipelineParticle,
        GraphicsPipelineCount,
    };

    enum ParticleBufferType
    {
        ParticleBufferDefault,
        ParticleBufferCount,
    };

    enum MeshType
    {
        MeshCube,
        MeshCubeWireframe,
        MeshCount,
    };

    SDL_GPUDevice* m_device;
    SDL_GPUGraphicsPipeline* m_graphicsPipelines[GraphicsPipelineCount];
    SDL_GPUTexture* m_textures[TextureCount];
    SDL_GPUSampler* m_samplers[SamplerCount];
    std::unordered_map<std::string, std::shared_ptr<Model>> m_models;
    std::array<ParticleBuffer, ParticleBufferCount> m_particleBuffers;
    std::array<Mesh, MeshCount> m_meshes;
    SDL_GPUCommandBuffer* m_commandBuffer;
    uint32_t m_width;
    uint32_t m_height;
    Camera m_camera;
};

}