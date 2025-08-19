#include <SDL3/SDL.h>
#include <SDLx_gpu/SDL_gpu.h>
#include <SDLx_model/SDL_model.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdint>
#include <string>

#include "Api.hpp"
#include "Assert.hpp"
#include "Camera.hpp"
#include "Renderer.hpp"

namespace
{

static constexpr int GridLines = 20;
static constexpr int GridSpacing = 20;

}

namespace Crobots
{

bool Renderer::Init()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        CROBOTS_LOG("Failed to initialize SDL: %s", SDL_GetError());
        return false;
    }
    m_window = SDL_CreateWindow("Crobots++", 960, 720, SDL_WINDOW_RESIZABLE);
    if (!m_window)
    {
        CROBOTS_LOG("Failed to create window: %s", SDL_GetError());
        return false;
    }
    m_device = SDLx_GPUCreateDevice(true);
    if (!m_device)
    {
        CROBOTS_LOG("Failed to create device: %s", SDL_GetError());;
        return false;
    }
    if (!SDL_ClaimWindowForGPUDevice(m_device, m_window))
    {
        CROBOTS_LOG("Failed to claim window: %s", SDL_GetError());
        return false;
    }
    m_renderer = SDLx_GPUCreateRenderer(m_device);
    if (!m_renderer)
    {
        CROBOTS_LOG("Failed to create renderer: %s", SDL_GetError());
        return false;
    }
    SDL_FlashWindow(m_window, SDL_FLASH_BRIEFLY);
    return true;
}

void Renderer::Quit()
{
    SDLx_GPUDestroyRenderer(m_renderer);
    SDL_ReleaseGPUTexture(m_device, m_colorTexture);
    SDL_ReleaseGPUTexture(m_device, m_depthTexture);
    SDL_ReleaseWindowFromGPUDevice(m_device, m_window);
    SDL_DestroyGPUDevice(m_device);
}

void Renderer::Present()
{
    SDL_GPUCommandBuffer* commandBuffer;
    SDL_GPUTexture* swapchainTexture;
    uint32_t width;
    uint32_t height;
    if (!SDLx_GPUBeginFrame(m_device, m_window, &commandBuffer, &swapchainTexture, &width, &height))
    {
        return;
    }
    if (m_camera.GetWidth() != width || m_camera.GetHeight() != height)
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
        m_camera.SetViewport(width, height);
    }
    m_camera.Update();
    /* TODO: obviously draw based on the arena in the future */
    for (int i = -GridLines; i <= GridLines; i++)
    {
        float a = i * GridSpacing;
        float b = GridLines * GridSpacing;
        SDLx_GPURenderLine3D(m_renderer, a, 0.0f, -b, a, 0.0f, b, 0xFFFFFFFF);
        SDLx_GPURenderLine3D(m_renderer, -b, 0.0f, a, b, 0.0f, a, 0xFFFFFFFF);
    }
    SDLx_GPUClear(commandBuffer, m_colorTexture, m_depthTexture);
    SDLx_GPUSubmitRenderer(m_renderer, commandBuffer, m_colorTexture, m_depthTexture,
        &m_camera.GetOrtho(), &m_camera.GetViewProj());
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

void Renderer::Draw(const std::string& path, float x, float y, float z, float yaw)
{
    SDLx_Model* model = SDLx_GPUGetModel(m_renderer, path.data(), SDLX_MODELTYPE_VOXOBJ);
    if (!model)
    {
        return;
    }
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, glm::vec3{x, y, z} - glm::vec3{0.0f, model->min.y, 0.0f});
    transform = glm::rotate(transform, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    // transform = glm::rotate(transform, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    // transform = glm::rotate(transform, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    // transform = glm::scale(transform, glm::vec3{1.0f, 1.0f, 1.0f});
    SDLx_GPURenderModel(m_renderer, path.data(), &transform, SDLX_MODELTYPE_VOXOBJ);
}

}