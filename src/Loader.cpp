#include <memory>

#include <SDL3/SDL.h>

#include "Crobots++/InternalRobotProxy.hpp"
#include "Loader.hpp"

namespace Crobots {

Loader::Loader(std::shared_ptr<Engine> engine)
: m_engine(engine)
{}

bool Loader::Load(const std::string& name, uint32_t id)
{
	CROBOTS_LOG("loading robot {}", name);
    // This should not be a path. Reject anything that is.
    if (name.size() == 0)
    {
        CROBOTS_LOG("Cannot load empty robot name");
        return false;
    }
    if ((name[0] == '/') || (name[0] == '.'))
    {
        CROBOTS_LOG("Path characters not permitted in robot name");
        return false;
    }
    std::string filename(name);
#if defined(SDL_PLATFORM_WIN32)
    filename += ".dll";
    filename = ".\\" + filename;
#elif defined(SDL_PLATFORM_APPLE)
    filename = "lib" + filename + ".dylib";
    filename = "./" + filename;
#else
    filename = "lib" + filename + ".so";
    filename = "./" + filename;
#endif
    // FIXME: Unload when done?
    SDL_SharedObject *plugin = SDL_LoadObject(filename.c_str());
    if (!plugin)
    {
        CROBOTS_LOG("SDL_LoadObject failed on {}", filename);
        return false;
    }

    // Cast SDL_FunctionPointer to the correct function type
    using GetRobotFunc = Crobots::IRobot* (*)(std::shared_ptr<InternalRobotProxy> proxy);
    GetRobotFunc fcn = reinterpret_cast<GetRobotFunc>(SDL_LoadFunction(plugin, "GetRobot"));
    if (!fcn)
    {
        CROBOTS_LOG("Failed to find entry point in library");
        return false;
    }

    auto proxy = std::make_shared<InternalRobotProxy>(id, m_engine);

    std::unique_ptr<Crobots::IRobot> robot(fcn(proxy));
    // FIXME: obsolete
    robot->SetId(id);
    m_robots.push_back(std::move(robot));

    return true;
}

std::vector<std::shared_ptr<Crobots::IRobot>>&& Loader::GetRobots()
{
    return std::move(m_robots);
}

}
