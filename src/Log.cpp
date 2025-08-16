#include <SDL3/SDL.h>

#include <string>

#include "Api.hpp"

namespace Crobots::Internal
{

void Log(const std::string& string)
{
    SDL_Log(string.data());
}

}