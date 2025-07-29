#include <SDL3/SDL.h>

#include "Log.hpp"
#include "Window.hpp"

namespace Crobots
{

bool Window::Create()
{
    SDL_SetAppMetadata("Crobots++", nullptr, nullptr);
    SDL_SetLogPriorities(SDL_LOG_PRIORITY_VERBOSE);
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        CROBOTS_LOG("Failed to initialize SDL: %s", SDL_GetError());
        return false;
    }
    window = SDL_CreateWindow("Crobots++", 960, 720, SDL_WINDOW_RESIZABLE);
    if (!window)
    {
        CROBOTS_LOG("Failed to create window: %s", SDL_GetError());
        return false;
    }
    SDL_FlashWindow(window, SDL_FLASH_BRIEFLY);
    return true;
}

void Window::Destroy()
{
    SDL_DestroyWindow(window);
    SDL_Quit();
}

SDL_Window* Window::GetHandle() const
{
    return window;
}

}