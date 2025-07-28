#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "Log.hpp"
#include "Renderer.hpp"
#include "Window.hpp"

using namespace Crobots;

static Window window;
static Renderer renderer;

SDL_AppResult SDLCALL SDL_AppInit(void** appstate, int argc, char** argv)
{
    if (!window.Create())
    {
        CROBOTS_LOG("Failed to create window");
        return SDL_APP_FAILURE;
    }
    if (!renderer.Create(window))
    {
        CROBOTS_LOG("Failed to create renderer");
        return SDL_APP_FAILURE;
    }
    return SDL_APP_CONTINUE;
}

void SDLCALL SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    renderer.Destroy(window);
    window.Destroy();
}

SDL_AppResult SDLCALL SDL_AppIterate(void* appstate)
{
    renderer.Present(window);
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDLCALL SDL_AppEvent(void* appstate, SDL_Event* event)
{
    switch (event->type)
    {
    case SDL_EVENT_QUIT:
        return SDL_APP_SUCCESS;
    case SDL_EVENT_KEY_DOWN:
        if (event->key.scancode == SDL_SCANCODE_ESCAPE)
        {
            return SDL_APP_SUCCESS;
        }
        break;
    }
    return SDL_APP_CONTINUE;
}