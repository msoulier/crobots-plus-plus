#include <SDL3/SDL.h>

#include "Log.hpp"
#include "Renderer.hpp"
#include "Window.hpp"

namespace Crobots
{
    
bool Renderer::CreateDevice()
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
    device = SDL_CreateGPUDeviceWithProperties(properties);
    if (!device)
    {
        CROBOTS_LOG("Failed to create device: %s", SDL_GetError());
        return false;
    }
    return true;
}

bool Renderer::Create(Window& window)
{
    if (!CreateDevice())
    {
        CROBOTS_LOG("Failed to create device");
        return false;
    }
    if (!SDL_ClaimWindowForGPUDevice(device, window.GetHandle()))
    {
        CROBOTS_LOG("Failed to claim window: %s", SDL_GetError());
        return false;
    }
    return true;
}

void Renderer::Destroy(Window& window)
{
    SDL_ReleaseWindowFromGPUDevice(device, window.GetHandle());
    SDL_DestroyGPUDevice(device);
}

void Renderer::Present(Window& window)
{

}

}
