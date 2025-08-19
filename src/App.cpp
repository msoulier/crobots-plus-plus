#include <SDL3/SDL.h>

#include "Crobots++/Log.hpp"
#include "Api.hpp"
#include "App.hpp"
#include "Loader.hpp"
#include "Renderer.hpp"
#include "Timer.hpp"

namespace Crobots
{

App::App()
    : m_engine{}
    , m_renderer{}
    , m_renderTimer{}
    , m_engineTimer{}
    , m_shouldQuit{false} {}

bool App::Init(const AppInfo& info)
{
    SDL_SetAppMetadata(info.title.data(), nullptr, nullptr);
    if (!m_renderer.Init())
    {
        CROBOTS_LOG("Failed to create renderer");
        return false;
    }
    m_renderTimer = Timer{16.6f};
    m_engineTimer = Timer{1000.0f};

    CROBOTS_LOG("Creating arena dimensions {} and {}", info.arenaX, info.arenaY);
    Arena arena(info.arenaX, info.arenaY);
    Loader loader;
	// FIXME: load the other robots
	if (! loader.Load(info.robot1_path))
	{
		std::cerr << "Failed to load " << info.robot1_path << std::endl;
		return false;
	}
    // FIXME: loop over command-line arguments and Load() each robot
    m_engine.Load(loader.GetRobots(), arena);
    return true;
}

bool App::ShouldQuit()
{
    return m_shouldQuit;
}

void App::Quit()
{
    m_renderer.Quit();
}

void App::Iterate()
{
    m_renderTimer.Tick();
    m_engineTimer.Tick();
    if (m_renderTimer.ShouldTick())
    {
        m_renderer.Present(m_engine);
    }
    if (m_engineTimer.ShouldTick())
    {
        m_engine.Tick();
    }
}

void App::Event(SDL_Event* event)
{
    switch (event->type)
    {
    case SDL_EVENT_QUIT:
        m_shouldQuit = true;
        break;
    }
}

}
