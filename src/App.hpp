#pragma once

#include <cstdint>
#include <SDL3/SDL.h>

#include <string_view>

#include "Api.hpp"
#include "Camera.hpp"
#include "Engine.hpp"
#include "Renderer.hpp"
#include "Timer.hpp"

namespace Crobots
{

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
    Renderer m_renderer;
    Timer m_renderTimer;
    Timer m_engineTimer;
    bool m_shouldQuit;
    Camera m_camera;
    std::shared_ptr<Engine> m_engine;
};

}
