#pragma once

#include <SDL3/SDL.h>

#define CROBOTS_LOG(...) SDL_Log(__VA_ARGS__)

namespace Crobots
{

void SetLogging(std::string logfilePath);
void ResetLogging();

}
