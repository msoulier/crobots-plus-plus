#include <SDL3/SDL.h>
#include <SDLx_gpu/SDL_gpu.h>
#include <SDLx_model/SDL_model.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdint>
#include <format>
#include <string>
#include <thread>
#include <chrono>

#include "Api.hpp"
#include "Arena.hpp"
#include "Assert.hpp"
#include "Camera.hpp"
#include "Engine.hpp"
#include "Renderer.hpp"

namespace
{

static constexpr int GridSpacing = 5;
static constexpr const char* FontPath = "RasterForgeRegular.ttf";

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

void Renderer::Present(const std::shared_ptr<Engine> engine, Camera& camera)
{
    SDL_GPUCommandBuffer* commandBuffer;
    SDL_GPUTexture* swapchainTexture;
    uint32_t width;
    uint32_t height;
    if (!SDLx_GPUBeginFrame(m_device, m_window, &commandBuffer, &swapchainTexture, &width, &height))
    {
        return;
    }
    if (camera.GetWidth() != width || camera.GetHeight() != height)
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
    }
    const Arena& arena = engine->GetArena();
    camera.SetCenter(arena.GetX() / 2, arena.GetY() / 2);
    camera.SetViewport(width, height);
    camera.Update();
    {
        int w = arena.GetX() / GridSpacing;
        int h = arena.GetY() / GridSpacing;
        for (int i = 0; i <= w; i++)
        {
            float a = i * GridSpacing;
            float b = w * GridSpacing;
            SDLx_GPURenderLine3D(m_renderer, arena.GetX() - a, 0.0f, 0.0f, arena.GetX() - a, 0.0f, b, 0xFFFFFFFF);
        }
        for (int i = 0; i <= h; i++)
        {
            float a = i * GridSpacing;
            float b = h * GridSpacing;
            SDLx_GPURenderLine3D(m_renderer, arena.GetX() - 0.0f, 0.0f, a, arena.GetX() - b, 0.0f, a, 0xFFFFFFFF);
        }
        for (int i = 0; i <= w; i++)
        for (int j = 0; j <= h; j++)
        {
            float a = i * GridSpacing;
            float b = j * GridSpacing;
            Draw(std::format("{} {}", i * GridSpacing, j * GridSpacing), arena.GetX() - a, b, 0xFFFFFFFF);
        }
        auto& robots = engine->GetRobots();
        for (auto& robot : robots)
        {
            Draw("default", arena.GetX() - robot->GetX(), 0.0f, robot->GetY(), IRobot::ToRadians(robot->GetFacing()), 0.1f);
            // Draw debug lines if debug is enabled.
            if (engine->DebugEnabled())
            {
                // The facing line
                Position facing = Engine::GetPositionAhead(robot->GetX(), robot->GetY(), robot->GetFacing(), 50.0f);
                SDLx_GPURenderLine3D(m_renderer, arena.GetX() - robot->GetX(), 0.0f, robot->GetY(),
                    arena.GetX() - facing.GetX(), 0.0f, facing.GetY(),
                    0xFF00FFFF);
                // The center scan line
                Position scandir = Engine::GetPositionAhead(robot->GetX(), robot->GetY(), robot->GetScanDir(), 60.0f);
                SDLx_GPURenderLine3D(m_renderer, arena.GetX() - robot->GetX(), 0.0f, robot->GetY(),
                    arena.GetX() - scandir.GetX(), 0.0f, scandir.GetY(),
                    0x00FFFFFF);
                float resolution = robot->GetResolution();
                // The right boundary of the scan
                Position scanright = Engine::GetPositionAhead(robot->GetX(),
                                                              robot->GetY(),
                                                              robot->GetScanDir()+(resolution/2),
                                                              60.0f);
                SDLx_GPURenderLine3D(m_renderer, arena.GetX() - robot->GetX(), 0.0f, robot->GetY(),
                    arena.GetX() - scanright.GetX(), 0.0f, scanright.GetY(),
                    0x00FFFFFF);
                // The left boundary of the scan
                Position scanleft = Engine::GetPositionAhead(robot->GetX(),
                                                             robot->GetY(),
                                                             robot->GetScanDir()-(resolution/2),
                                                             60.0f);
                SDLx_GPURenderLine3D(m_renderer, arena.GetX() - robot->GetX(), 0.0f, robot->GetY(),
                    arena.GetX() - scanleft.GetX(), 0.0f, scanleft.GetY(),
                    0x00FFFFFF);
                const std::vector<std::unique_ptr<ContactDetails>>& contacts = robot->GetContacts();
                for (const std::unique_ptr<ContactDetails>& contact : contacts) {
                    CROBOTS_LOG("contact at bearing {}, range {}", contact->m_bearing, contact->m_range);
                    CROBOTS_LOG("from {} {} to {} {}", contact->m_fromx, contact->m_fromy,
                                                       contact->m_tox, contact->m_toy);
                    SDLx_GPURenderLine3D(m_renderer, arena.GetX() - contact->m_fromx, 0.0f, contact->m_fromy,
                        arena.GetX() - contact->m_tox, 0.0f, contact->m_toy,
                        0x00FFFFFF);
                }
            }
        }
    }
    SDLx_GPUClear(commandBuffer, m_colorTexture, m_depthTexture);
    SDLx_GPUSubmitRenderer(m_renderer, commandBuffer, m_colorTexture,
        m_depthTexture, &camera.GetMatrix2D(), &camera.GetMatrix3D());
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

void Renderer::Draw(const std::string& path, float x, float y, float z, float yaw, float scale)
{
    SDLx_Model* model = SDLx_GPUGetModel(m_renderer, path.data(), SDLX_MODELTYPE_VOXOBJ);
    if (!model)
    {
        return;
    }
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, glm::vec3{x, y, z} - glm::vec3{0.0f, model->min.y * scale, 0.0f});
    transform = glm::rotate(transform, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    // transform = glm::rotate(transform, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    // transform = glm::rotate(transform, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    transform = glm::scale(transform, glm::vec3{scale});
    SDLx_GPURenderModel(m_renderer, path.data(), &transform, SDLX_MODELTYPE_VOXOBJ);
}

void Renderer::Draw(const std::string& text, float x, float y, Uint32 color)
{
    static constexpr float Scale = 0.05f;
    static constexpr float Pitch = glm::pi<float>() * 3.0f / 2.0f;
    static constexpr float Roll = glm::pi<float>();
    static constexpr int Size = 16;
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, glm::vec3{x, 0.0f, y});
    // transform = glm::rotate(transform, Yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    transform = glm::rotate(transform, Pitch, glm::vec3(1.0f, 0.0f, 0.0f));
    transform = glm::rotate(transform, Roll, glm::vec3(0.0f, 0.0f, 1.0f));
    transform = glm::scale(transform, glm::vec3{Scale});
    SDLx_GPURenderText3D(m_renderer, FontPath, text.data(), &transform, Size, color);
}

}
