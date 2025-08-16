#include <SDL3/SDL.h>
#include <SDLx_gpu/SDL_gpu.h>
#include <SDLx_model/SDL_model.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include "Assert.hpp"
#include "Camera.hpp"
#include "DebugGroup.hpp"
#include "Log.hpp"
#include "Math.hpp"
#include "Pipeline.hpp"
#include "Renderer.hpp"
#include "Window.hpp"

namespace Crobots
{

bool Renderer::Create(Window& window)
{
    m_device = SDLx_GPUCreateDevice();
    if (!m_device)
    {
        return false;
    }
    if (!SDL_ClaimWindowForGPUDevice(m_device, window.GetHandle()))
    {
        CROBOTS_LOG("Failed to claim window: %s", SDL_GetError());
        return false;
    }
    m_modelVoxObjPipeline = CreateModelVoxObjPipeline(m_device, window);
    if (!m_modelVoxObjPipeline)
    {
        return false;
    }
    m_nearestSampler = SDLx_GPUCreateNearestSampler(m_device);
    if (!m_nearestSampler)
    {
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

    /* TODO: remove */
    m_models["default"] = SDLx_ModelLoad(m_device, copyPass, "default", SDLX_MODELTYPE_VOXOBJ);

    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(commandBuffer);
    return true;
}

void Renderer::Destroy(Window& window)
{
    for (auto& [name, model] : m_models)
    {
        SDLx_ModelDestroy(m_device, model);
    }
    SDL_ReleaseGPUSampler(m_device, m_nearestSampler);
    SDL_ReleaseGPUTexture(m_device, m_colorTexture);
    SDL_ReleaseGPUTexture(m_device, m_depthTexture);
    SDL_ReleaseGPUGraphicsPipeline(m_device, m_modelVoxObjPipeline);
    SDL_ReleaseWindowFromGPUDevice(m_device, window.GetHandle());
    SDL_DestroyGPUDevice(m_device);
}

void Renderer::Present(Window& window)
{
    SDL_GPUCommandBuffer* commandBuffer;
    SDL_GPUTexture* swapchainTexture;
    uint32_t width;
    uint32_t height;
    if (!SDLx_GPUBeginFrame(m_device, window.GetHandle(), &commandBuffer, &swapchainTexture, &width, &height))
    {
        return;
    }
    if (m_width != width || m_height != height)
    {
        SDL_ReleaseGPUTexture(m_device, m_colorTexture);
        SDL_ReleaseGPUTexture(m_device, m_depthTexture);
        m_colorTexture = SDLx_GPUCreateColorTexture(m_device, width, height, SDL_GPU_TEXTUREUSAGE_SAMPLER);
        m_depthTexture = SDLx_GPUCreateDepthTexture(m_device, width, height, 0);
        if (!m_colorTexture || !m_depthTexture)
        {
            SDL_Log("Failed to create texture(s)");
            SDL_SubmitGPUCommandBuffer(commandBuffer);
            return;
        }
        m_width = width;
        m_height = height;
    }
    RenderModels(commandBuffer);
    {
        SDL_GPUBlitInfo info{};
        info.source.texture = m_colorTexture;
        info.source.w = width;
        info.source.h = height;
        info.destination.texture = swapchainTexture;
        info.destination.w = width;
        info.destination.h = height;
        SDL_BlitGPUTexture(commandBuffer, &info);
    }
    SDL_SubmitGPUCommandBuffer(commandBuffer);
}

void Renderer::RenderModels(SDL_GPUCommandBuffer* commandBuffer)
{
    CROBOTS_DEBUG_GROUP(commandBuffer);
    SDL_GPUColorTargetInfo colorInfo{};
    colorInfo.texture = m_colorTexture;
    colorInfo.load_op = SDL_GPU_LOADOP_CLEAR;
    colorInfo.store_op = SDL_GPU_STOREOP_STORE;
    colorInfo.cycle = true;
    SDL_GPUDepthStencilTargetInfo depthInfo{};
    depthInfo.texture = m_depthTexture;
    depthInfo.load_op = SDL_GPU_LOADOP_CLEAR;
    depthInfo.stencil_load_op = SDL_GPU_LOADOP_CLEAR;
    depthInfo.store_op = SDL_GPU_STOREOP_STORE;
    depthInfo.clear_depth = 1.0f;
    depthInfo.cycle = true;
    SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, &colorInfo, 1, &depthInfo);
    if (!renderPass)
    {
        CROBOTS_LOG("Failed to begin render pass: %s", SDL_GetError());
        return;
    }

    /* TODO: remove */
    std::vector<SDLx_Model*> models{m_models["default"]};

    for (SDLx_Model* model : models)
    {
        switch (model->type)
        {
        case SDLX_MODELTYPE_VOXOBJ:
            RenderModelVoxObj(commandBuffer, renderPass, model);
            break;
        default:
            CROBOTS_ASSERT(false);
        }
    }

    SDL_EndGPURenderPass(renderPass);
}

void Renderer::RenderModelVoxObj(SDL_GPUCommandBuffer* commandBuffer, SDL_GPURenderPass* renderPass, SDLx_Model* model)
{
    /* TODO: remove */
    glm::vec3 position = glm::vec3(0.0f, 0.0f, -100.0f);
    glm::vec3 rotation = glm::vec3(0.5f, 0.5f, 0.0f);
    glm::mat4 matrix = CreateModelMatrix(position, rotation);
    m_camera.SetViewport(glm::vec2(m_width, m_height));
    m_camera.Update();

    SDL_BindGPUGraphicsPipeline(renderPass, m_modelVoxObjPipeline);
    SDL_PushGPUVertexUniformData(commandBuffer, 0, &m_camera.GetViewProj(), 64);
    SDL_PushGPUVertexUniformData(commandBuffer, 1, &matrix, 64);
    SDL_GPUBufferBinding vertexBufferBinding{};
    SDL_GPUBufferBinding indexBufferBinding{};
    SDL_GPUTextureSamplerBinding paletteTextureBinding{};
    vertexBufferBinding.buffer = model->vox_obj.vertex_buffer;
    indexBufferBinding.buffer = model->vox_obj.index_buffer;
    paletteTextureBinding.sampler = m_nearestSampler;
    paletteTextureBinding.texture = model->vox_obj.palette_texture;
    SDL_BindGPUVertexBuffers(renderPass, 0, &vertexBufferBinding, 1);
    SDL_BindGPUIndexBuffer(renderPass, &indexBufferBinding, model->vox_obj.index_element_size);
    SDL_BindGPUFragmentSamplers(renderPass, 0, &paletteTextureBinding, 1);
    SDL_DrawGPUIndexedPrimitives(renderPass, model->vox_obj.num_indices, 1, 0, 0, 0);
}

}