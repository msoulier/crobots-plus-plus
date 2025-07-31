#include <SDL3/SDL.h>

#include "App.hpp"
#include "Log.hpp"
#include "Renderer.hpp"
#include "Timer.hpp"
#include "Window.hpp"

namespace Crobots
{

App::App()
    : m_window{}
    , m_renderer{}
    , m_renderTimer{}
    , m_engineTimer{}
    , m_shouldQuit{false}
{
}

bool App::Init(const AppInfo& info)
{
    SDL_SetLogPriorities(SDL_LOG_PRIORITY_VERBOSE);
    SDL_SetAppMetadata(info.title.data(), nullptr, nullptr);
    if (!m_window.Create(info.title))
    {
        CROBOTS_LOG("Failed to create window");
        return false;
    }
    if (!m_renderer.Create(m_window))
    {
        CROBOTS_LOG("Failed to create renderer");
        return false;
    }
    m_renderTimer = Timer{16.6f};
    m_engineTimer = Timer{1000.0f};
}

bool App::ShouldQuit()
{
    return m_shouldQuit;
}

void App::Quit()
{
    m_renderer.Destroy(m_window);
    m_window.Destroy();
}

void App::Iterate()
{
    m_renderTimer.Tick();
    m_engineTimer.Tick();
    if (m_renderTimer.ShouldTick())
    {
        m_renderer.Present(m_window);
    }
    if (m_engineTimer.ShouldTick())
    {
        // FIXME: engine
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
