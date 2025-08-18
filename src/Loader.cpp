#include <memory>

#include <SDL3/SDL.h>

#include "Loader.hpp"

namespace Crobots {

bool Loader::Load(const std::string& path)
{
	CROBOTS_LOG("loading robot at path {}", path);
#if defined(SDL_PLATFORM_WIN32)
    SDL_SharedObject *plugin = SDL_LoadObject("./Doofus.dll");
#elif defined(SDL_PLATFORM_APPLE)
    SDL_SharedObject *plugin = SDL_LoadObject("./libDoofus.dylib");
#else
    SDL_SharedObject *plugin = SDL_LoadObject("./libDoofus.so");
#endif
    if (!plugin)
    {
        CROBOTS_LOG("SDL_LoadObject failed on {}", path);
        return false;
    }

    // Cast SDL_FunctionPointer to the correct function type
    using GetRobotFunc = Crobots::IRobot* (*)();
    GetRobotFunc fcn = reinterpret_cast<GetRobotFunc>(SDL_LoadFunction(plugin, "GetRobot"));
    if (!fcn)
    {
        CROBOTS_LOG("Failed to find entry point in library");
        return false;
    }

    std::unique_ptr<Crobots::IRobot> robot(fcn());
    m_robots.push_back(std::move(robot));
    return true;
}

std::vector<std::unique_ptr<Crobots::IRobot>>&& Loader::GetRobots()
{
    return std::move(m_robots);
}

}
