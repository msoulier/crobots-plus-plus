#pragma once

#include <SDL3/SDL.h>

namespace Crobots
{

class Window
{
public:
    bool Create();
    void Destroy();
    SDL_Window* GetHandle() const;

private:
    SDL_Window* m_window;
};

}