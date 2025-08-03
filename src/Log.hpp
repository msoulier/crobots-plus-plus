/* TODO: move into include/Crobots++ to provide an interface for robots to log from */

#pragma once

#include <SDL3/SDL.h>

#include <string_view>

#define CROBOTS_LOG(...) SDL_Log(__VA_ARGS__)

namespace Crobots
{

void SetLogging(const std::string_view& logfilePath);
void ResetLogging();

}
