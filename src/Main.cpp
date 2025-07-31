#include <CLI/CLI.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "App.hpp"

/* TODO: switch to callbacks when resize slowdowns on Vulkan get fixed */
int main(int argc, char** argv)
{
    Crobots::AppInfo info{};
    info.title = "Crobots++";
    /* TODO: add args to AppInfo */
    Crobots::App app{};
    if (!app.Init(info))
    {
        return 1;
    }
    while (!app.ShouldQuit())
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            app.Event(&event);
        }
        app.Iterate();
    }
    app.Quit();
    return 0;
}