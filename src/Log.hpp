#pragma once

#include <SDL3/SDL.h>

#include <string_view>

#define CROBOTS_LOG(...) SDL_Log(__VA_ARGS__)

namespace Crobots
{

void LogInit(const std::string_view& path);
void LogQuit();

}