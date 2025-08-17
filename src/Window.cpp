#include <SDL3/SDL.h>

#include <string_view>

#include "Api.hpp"
#include "Window.hpp"

namespace Crobots
{

bool Window::Create(const std::string_view& title)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        CROBOTS_LOG("Failed to initialize SDL: %s", SDL_GetError());
        return false;
    }
    m_window = SDL_CreateWindow(title.data(), 960, 720, SDL_WINDOW_RESIZABLE);
    if (!m_window)
    {
        CROBOTS_LOG("Failed to create window: %s", SDL_GetError());
        return false;
    }
    SDL_FlashWindow(m_window, SDL_FLASH_BRIEFLY);
    return true;
}

void Window::Destroy()
{
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

SDL_Window* Window::GetHandle() const
{
    return m_window;
}

}