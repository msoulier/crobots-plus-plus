#include <vector>

#include "Engine.hpp"
#include "Log.hpp"
#include "Arena.hpp"

namespace Crobots
{

void Engine::Tick()
{
    CROBOTS_LOG("Crobots::Engine::Tick()");
}

void Engine::Load(std::vector<std::unique_ptr<Crobots::IRobot>>&& robots, Crobots::Arena arena)
{
    m_robots = std::move(robots);
    m_arena = arena;
}

}
