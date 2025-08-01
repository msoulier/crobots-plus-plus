#include <SDL3/SDL.h>

#include "Log.hpp"

static SDL_IOStream* logFile;

static void LogCallback(void* data, int category, SDL_LogPriority priority, const char* string)
{
    SDL_GetDefaultLogOutputFunction()(data, category, priority, string);
    if (logFile)
    {
        SDL_WriteIO(logFile, string, SDL_strlen(string));
        SDL_WriteU8(logFile, '\n');
        SDL_FlushIO(logFile);
    }
}

namespace Crobots
{

void SetLogging()
{
    SDL_SetLogPriorities(SDL_LOG_PRIORITY_VERBOSE);
    logFile = SDL_IOFromFile("crobots++.log", "w");
    if (!logFile)
    {
        CROBOTS_LOG("Failed to open log file: %s", SDL_GetError());
        return;
    }
    SDL_SetLogOutputFunction(LogCallback, logFile);
}

void ResetLogging()
{
    SDL_ResetLogPriorities();
    SDL_CloseIO(logFile);
    logFile = nullptr;
}

}