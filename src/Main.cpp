#include <CLI/CLI.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <string>

#include "Api.hpp"
#include "App.hpp"

// Verbose logging.
static bool verbose = false;
static uint32_t arenaX = 1000;
static uint32_t arenaY = 1000;
static std::string logFile{"crobots++.log"};
static SDL_IOStream* logFileHandle;

static void LogCallback(void* data, int category, SDL_LogPriority priority, const char* string)
{
    SDL_GetDefaultLogOutputFunction()(data, category, priority, string);
    if (logFileHandle)
    {
        SDL_WriteIO(logFileHandle, string, SDL_strlen(string));
        SDL_WriteU8(logFileHandle, '\n');
        SDL_FlushIO(logFileHandle);
    }
}

// https://github.com/CLIUtils/CLI11 for CLI
static bool ParseOptions(int argc, char** argv, Crobots::AppInfo& info)
{
    /* TODO: add args to AppInfo */

    CLI::App parser{"Crobots++: <arguments>"};
    argv = parser.ensure_utf8(argv);

    parser.add_flag("-v,--verbose", verbose, "Verbose logging");
    parser.add_option("-x,--arena-x", arenaX, "Arena X dimension (default 1000)");
    parser.add_option("-y,--arena-y", arenaY, "Arena Y dimension (default 1000)");
    parser.add_option("-l,--logfile", logFile, "Path to logfile (default crobots++.log)");

    try
    {
        parser.parse(argc, argv);
    }
    catch (const CLI::ParseError& e)
    {
        parser.exit(e);
        return false;
    }

    info.arenaX = arenaX;
    info.arenaY = arenaY;

    return true;
}

/* TODO: switch to callbacks when resize slowdowns on Vulkan get fixed */
int main(int argc, char** argv)
{
    Crobots::AppInfo info{};
    info.title = "Crobots++";
    if (!ParseOptions(argc, argv, info))
    {
        return 1;
    }
    SDL_SetLogPriorities(SDL_LOG_PRIORITY_VERBOSE);
    logFileHandle = SDL_IOFromFile(logFile.data(), "w");
    if (!logFileHandle)
    {
        CROBOTS_LOG("Failed to open log file: %s", SDL_GetError());
    }
    SDL_SetLogOutputFunction(LogCallback, logFileHandle);
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
    SDL_ResetLogPriorities();
    SDL_SetLogOutputFunction(SDL_GetDefaultLogOutputFunction(), nullptr);
    SDL_CloseIO(logFileHandle);
    return 0;
}
