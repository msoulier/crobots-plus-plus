#include <SDL3/SDL.h>

#include <string_view>

#include "Log.hpp"

static SDL_IOStream* logFile;

static void LogCallback(void* data, int category, SDL_LogPriority priority, const char* string)
{
    SDL_GetDefaultLogOutputFunction()(data, category, priority, string);
    if (logFile)
    {
        SDL_IOprintf(logFile, "%s\n", string);
    }
}

namespace Crobots
{

void LogInit(const std::string_view& path)
{
    SDL_SetLogPriorities(SDL_LOG_PRIORITY_VERBOSE);
    logFile = SDL_IOFromFile(path.data(), "w");
    if (!logFile)
    {
        CROBOTS_LOG("Failed to open log file: %s", SDL_GetError());
        return;
    }
    SDL_SetLogOutputFunction(LogCallback, logFile);
}

void LogQuit()
{
    SDL_SetLogOutputFunction(SDL_GetDefaultLogOutputFunction(), nullptr);
    SDL_CloseIO(logFile);
}

}