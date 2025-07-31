#include <SDL3/SDL.h>

#include <cstdint>

#include "Log.hpp"
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
    , m_commandBuffer{nullptr}
    , m_width{0}
    , m_height{0}
{
}

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
    if (!CreatePipelines(window))
    {
        CROBOTS_LOG("Failed to create pipelines");
        return false;
    }
    return true;
}

void Renderer::Destroy(Window& window)
{
    if (m_commandBuffer)
    {
        SDL_SubmitGPUCommandBuffer(m_commandBuffer);
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

}