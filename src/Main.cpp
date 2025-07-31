#include <CLI/CLI.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "App.hpp"
#include "Log.hpp"

// Verbose logging.
static bool verbose = false;
static uint32_t arenaX = 1000;
static uint32_t arenaY = 1000;

// https://github.com/CLIUtils/CLI11 for CLI
int ParseOptions(int argc, char** argv)
{
    CLI::App parser{"Crobots++: <arguments>"};
    argv = parser.ensure_utf8(argv);

    parser.add_flag("-v,--verbose", verbose, "Verbose logging");
    parser.add_option("-x,--arena-x", arenaX, "Arena X dimension (default 1000)");
    parser.add_option("-y,--arena-y", arenaY, "Arena Y dimension (default 1000)");

    try
    {
        parser.parse(argc, argv);
    }
    catch (const CLI::ParseError& e)
    {
        parser.exit(e);
        return 1;
    }

    CROBOTS_LOG("argc is %d after parse", argc);
    return 0;
}

/* TODO: switch to callbacks when resize slowdowns on Vulkan get fixed */
int main(int argc, char** argv)
{
    if (ParseOptions(argc, argv) != 0)
    {
        return 1;
    }
    Crobots::AppInfo info{};
    info.title = "Crobots++";
    info.logPath = "crobots.log";
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