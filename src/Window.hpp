#pragma once

#include <SDL3/SDL.h>

#include <string_view>

namespace Crobots
{

class Window
{
public:
    bool Create(const std::string_view& title);
    void Destroy();
    SDL_Window* GetHandle() const;

private:
    SDL_Window* m_window;
};

}