#pragma once

#include <cstdint>
#include <SDL3/SDL.h>

#include <string_view>

#include "Camera.hpp"
#include "Engine.hpp"
#include "Renderer.hpp"
#include "Timer.hpp"

namespace Crobots
{

struct AppInfo
{
    std::string_view title;
    uint32_t arenaX;
    uint32_t arenaY;
	uint32_t nrobots;
	std::string robot1_path;
	std::string robot2_path;
	std::string robot3_path;
	std::string robot4_path;
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
    Engine m_engine;
    Renderer m_renderer;
    Camera m_camera;
    Timer m_renderTimer;
    Timer m_engineTimer;
    bool m_shouldQuit;
};

}
