#pragma once

#include <SDL3/SDL.h>

#include <string_view>

#include "Renderer.hpp"
#include "Timer.hpp"
#include "Window.hpp"

namespace Crobots
{

struct AppInfo
{
    std::string_view title;
    /* TODO: args */
};

class App
{
public:
    App();
    bool Init(const AppInfo& info);
    bool ShouldQuit();
    void Quit();
    void Iterate();
    void Event(SDL_Event* event);

private:
    Window m_window;
    Renderer m_renderer;
    Timer m_renderTimer;
    Timer m_engineTimer;
    bool m_shouldQuit;
};

}