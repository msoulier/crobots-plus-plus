#include <SDL3/SDL.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include "Assert.hpp"
#include "Camera.hpp"
#include "DebugGroup.hpp"
#include "Log.hpp"
#include "Math.hpp"
#include "Model.hpp"
#include "ModelVoxObj.hpp"
#include "ParticleBuffer.hpp"
#include "Pipeline.hpp"
#include "Renderer.hpp"
#include "Texture.hpp"
#include "Window.hpp"

namespace Crobots
{

Renderer::Renderer()
    : m_device{nullptr}
    , m_graphicsPipelines{}
    , m_textures{}
    , m_samplers{}
    , m_models{}
    , m_particleBuffers{}
    , m_meshes{}
    , m_commandBuffer{nullptr}
    , m_width{0}
    , m_height{0}
    , m_camera{} {}

bool Renderer::Create(Window& window)
{
    if (!CreateDevice(window))
    {
        CROBOTS_LOG("Failed to create device");
        return false;
    }
    if (!SDL_ClaimWindowForGPUDevice(m_device, window.GetHandle()))
    {
        CROBOTS_LOG("Failed to claim window: %s", SDL_GetError());
        return false;
    }
    SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(m_device);
    if (!commandBuffer)
    {
        CROBOTS_LOG("Failed to acquire command buffer: %s", SDL_GetError());
        return false;
    }
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);
    if (!copyPass)
    {
        CROBOTS_LOG("Failed to begin copy pass: %s", SDL_GetError());
        return false;
    }
    if (!CreatePipelines(window))
    {
        CROBOTS_LOG("Failed to create pipelines");
        return false;
    }
    if (!CreateSamplers(window))
    {
        CROBOTS_LOG("Failed to create samplers");
        return false;
    }
    if (!CreateMeshes(window, copyPass))
    {
        CROBOTS_LOG("Failed to create meshes");
        return false;
    }
    if (!CreateParticleBuffers(window, copyPass))
    {
        CROBOTS_LOG("Failed to create particle buffers");
        return false;
    }

    /* TODO: remove */
    m_models["default"] = Model::Create(m_device, copyPass, "default");

    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(commandBuffer);
    return true;
}

void Renderer::Destroy(Window& window)
{
    if (m_commandBuffer)
    {
        SDL_SubmitGPUCommandBuffer(m_commandBuffer);
    }
    for (Mesh& mesh : m_meshes)
    {
        mesh.Destroy(m_device);
    }
    for (ParticleBuffer& particleBuffer : m_particleBuffers)
    {
        particleBuffer.Destroy(m_device);
    }
    for (std::pair<const std::string, std::shared_ptr<Model>>& item : m_models)
    {
        item.second->Destroy(m_device);
    }
    for (int i = 0; i < SamplerCount; i++)
    {
        SDL_ReleaseGPUSampler(m_device, m_samplers[i]);
    }
    for (int i = 0; i < TextureCount; i++)
    {
        SDL_ReleaseGPUTexture(m_device, m_textures[i]);
    }
    for (int i = 0; i < GraphicsPipelineCount; i++)
    {
        SDL_ReleaseGPUGraphicsPipeline(m_device, m_graphicsPipelines[i]);
    }
    SDL_ReleaseWindowFromGPUDevice(m_device, window.GetHandle());
    SDL_DestroyGPUDevice(m_device);
}

void Renderer::Present(Window& window)
{
    if (!m_commandBuffer)
    {
        m_commandBuffer = SDL_AcquireGPUCommandBuffer(m_device);
        if (!m_commandBuffer)
        {
            CROBOTS_LOG("Failed to acquire command buffer: %s", SDL_GetError());
            return;
        }
    }
    SDL_GPUTexture* swapchainTexture = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
    if (!SDL_AcquireGPUSwapchainTexture(m_commandBuffer, window.GetHandle(), &swapchainTexture, &width, &height))
    {
        /* NOTE: not an error since we're synchronizing externally */
        return;
    }
    if (!swapchainTexture || !width || !height)
    {
        /* NOTE: not an error. happens on minimize */
        return;
    }
    if ((width != m_width || height != m_height) && !ResizeTextures(width, height))
    {
        CROBOTS_LOG("Failed to resize texture(s)");
        SDL_SubmitGPUCommandBuffer(m_commandBuffer);
        m_commandBuffer = nullptr;
        return;
    }
    RenderModels(swapchainTexture);
    SDL_SubmitGPUCommandBuffer(m_commandBuffer);
    m_commandBuffer = nullptr;
}

bool Renderer::CreateDevice(Window& window)
{
    SDL_PropertiesID properties = SDL_CreateProperties();
    SDL_SetBooleanProperty(properties, SDL_PROP_GPU_DEVICE_CREATE_PREFERLOWPOWER_BOOLEAN, true);
#ifndef NDEBUG
    SDL_SetBooleanProperty(properties, SDL_PROP_GPU_DEVICE_CREATE_DEBUGMODE_BOOLEAN, true);
    SDL_SetBooleanProperty(properties, SDL_PROP_GPU_DEVICE_CREATE_VERBOSE_BOOLEAN, true);
#endif
#if SDL_PLATFORM_WIN32
    /* TODO: waiting on https://github.com/libsdl-org/SDL/issues/12056 */
#ifndef NDEBUG
    SDL_SetBooleanProperty(properties, SDL_PROP_GPU_DEVICE_CREATE_SHADERS_SPIRV_BOOLEAN, true);
#else
    SDL_SetBooleanProperty(properties, SDL_PROP_GPU_DEVICE_CREATE_SHADERS_DXIL_BOOLEAN, true);
#endif
#elif SDL_PLATFORM_APPLE
    SDL_SetBooleanProperty(properties, SDL_PROP_GPU_DEVICE_CREATE_SHADERS_METAL_BOOLEAN, true);
#else
    SDL_SetBooleanProperty(properties, SDL_PROP_GPU_DEVICE_CREATE_SHADERS_SPIRV_BOOLEAN, true);
#endif
    m_device = SDL_CreateGPUDeviceWithProperties(properties);
    if (!m_device)
    {
        CROBOTS_LOG("Failed to create device: %s", SDL_GetError());
        return false;
    }
    return true;
}

bool Renderer::CreatePipelines(Window& window)
{
    m_graphicsPipelines[GraphicsPipelineModelVoxObj] = CreateModelVoxObjPipeline(m_device, window);
    m_graphicsPipelines[GraphicsPipelineParticle] = CreateParticlePipeline(m_device, window);
    for (int i = GraphicsPipelineCount - 1; i >= 0; i--)
    {
        if (!m_graphicsPipelines[i])
        {
            SDL_Log("Failed to create graphics pipeline: %d", i);
            return false;
        }
    }
    return true;
}

bool Renderer::CreateSamplers(Window& window)
{
    SDL_GPUSamplerCreateInfo info{};
    info.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    info.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    info.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    info.min_filter = SDL_GPU_FILTER_NEAREST;
    info.mag_filter = SDL_GPU_FILTER_NEAREST;
    info.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
    m_samplers[SamplerNearest] = SDL_CreateGPUSampler(m_device, &info);
    for (int i = SamplerCount - 1; i >= 0; i--)
    {
        if (!m_samplers[i])
        {
            SDL_Log("Failed to create sampler: %d, %s", i, SDL_GetError());
            return false;
        }
    }
    return true;
}

bool Renderer::CreateMeshes(Window& window, SDL_GPUCopyPass* copyPass)
{
    std::optional<Mesh> cubeMesh = Mesh::CreateCubeMesh(m_device, copyPass);
    if (!cubeMesh)
    {
        CROBOTS_LOG("Failed to create cube mesh");
        return false;
    }
    std::optional<Mesh> cubeWireframeMesh = Mesh::CreateCubeWireframeMesh(m_device, copyPass);
    if (!cubeWireframeMesh)
    {
        CROBOTS_LOG("Failed to create cube wireframe mesh");
        return false;
    }
    m_meshes[MeshCube] = cubeMesh.value();
    m_meshes[MeshCubeWireframe] = cubeWireframeMesh.value();
    return true;
}

bool Renderer::CreateParticleBuffers(Window& window, SDL_GPUCopyPass* copyPass)
{
    if (!m_particleBuffers[ParticleBufferDefault].Create(m_device, copyPass, m_meshes[MeshCube].GetIndexCount()))
    {
        CROBOTS_LOG("Failed to create particle buffer: %s", SDL_GetError());
        return false;
    }
    return true;
}

bool Renderer::ResizeTextures(uint32_t width, uint32_t height)
{
    for (int i = 0; i < TextureCount; i++)
    {
        SDL_ReleaseGPUTexture(m_device, m_textures[i]);
        m_textures[i] = nullptr;
    }
    m_textures[TextureDepth] = CreateDepthTexture(m_device, width, height);
    for (int i = TextureCount - 1; i >= 0; i--)
    {
        if (!m_textures[i])
        {
            SDL_Log("Failed to create texture: %d, %s", i, SDL_GetError());
            return false;
        }
    }
    m_width = width;
    m_height = height;
    return true;
}

void Renderer::RenderModels(SDL_GPUTexture* colorTexture)
{
    CROBOTS_DEBUG_GROUP(m_commandBuffer);
    SDL_GPUColorTargetInfo colorInfo{};
    colorInfo.texture = colorTexture;
    colorInfo.load_op = SDL_GPU_LOADOP_CLEAR;
    colorInfo.store_op = SDL_GPU_STOREOP_STORE;
    colorInfo.cycle = true;
    SDL_GPUDepthStencilTargetInfo depthInfo{};
    depthInfo.texture = m_textures[TextureDepth];
    depthInfo.load_op = SDL_GPU_LOADOP_CLEAR;
    depthInfo.stencil_load_op = SDL_GPU_LOADOP_CLEAR;
    depthInfo.store_op = SDL_GPU_STOREOP_STORE;
    depthInfo.clear_depth = 1.0f;
    depthInfo.cycle = true;
    SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(m_commandBuffer, &colorInfo, 1, &depthInfo);
    if (!renderPass)
    {
        CROBOTS_LOG("Failed to begin render pass: %s", SDL_GetError());
        return;
    }

    /* TODO: remove */
    std::vector<std::shared_ptr<Model>> models{m_models["default"]};

    for (const std::shared_ptr<Model>& model : models)
    {
        switch (model->GetType())
        {
        case ModelType::VoxObj:
            RenderModelVoxObj(renderPass, std::dynamic_pointer_cast<ModelVoxObj>(model));
            break;
        default:
            CROBOTS_ASSERT(false);
        }
    }

    SDL_EndGPURenderPass(renderPass);
}

void Renderer::RenderModelVoxObj(SDL_GPURenderPass* renderPass, const std::shared_ptr<ModelVoxObj>& model)
{
    /* TODO: remove */
    glm::vec3 position = glm::vec3(0.0f, 0.0f, -100.0f);
    glm::vec3 rotation = glm::vec3(0.5f, 0.5f, 0.0f);
    glm::mat4 matrix = CreateModelMatrix(position, rotation);
    m_camera.SetViewport(glm::vec2(m_width, m_height));
    m_camera.Update();

    SDL_BindGPUGraphicsPipeline(renderPass, m_graphicsPipelines[GraphicsPipelineModelVoxObj]);
    SDL_PushGPUVertexUniformData(m_commandBuffer, 0, &m_camera.GetViewProj(), 64);
    SDL_PushGPUVertexUniformData(m_commandBuffer, 1, &matrix, 64);
    SDL_GPUBufferBinding vertexBufferBinding{};
    SDL_GPUBufferBinding indexBufferBinding{};
    SDL_GPUTextureSamplerBinding paletteTextureBinding{};
    vertexBufferBinding.buffer = model->GetVertexBuffer();
    indexBufferBinding.buffer = model->GetIndexBuffer();
    paletteTextureBinding.sampler = m_samplers[SamplerNearest];
    paletteTextureBinding.texture = model->GetPaletteTexture();
    SDL_BindGPUVertexBuffers(renderPass, 0, &vertexBufferBinding, 1);
    SDL_BindGPUIndexBuffer(renderPass, &indexBufferBinding, model->GetIndexElementSize());
    SDL_BindGPUFragmentSamplers(renderPass, 0, &paletteTextureBinding, 1);
    SDL_DrawGPUIndexedPrimitives(renderPass, model->GetIndexCount(), 1, 0, 0, 0);
}

}