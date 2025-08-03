#include <cstdlib>
#include <ctime>
#include <vector>

#include "Engine.hpp"
#include "Log.hpp"
#include "Arena.hpp"

namespace Crobots
{

void Engine::Tick()
{
    CROBOTS_LOG("Crobots::Engine::Tick()");
    for (std::unique_ptr<Crobots::IRobot>& robot : m_robots) {
        // Run each robot through a tick.
        robot->Tick();
        // Update the position of each robot based on its velocity
        void MoveRobots();
        // Update the velocity (ie. speed and facing) of each robot
        void AccelRobots();
        // Check for any loss of control (ie. skidding) - future item

        // Update the position of any shots in flight
        void MoveShotsInFlight();

        // Fire any direct fire weapons that have zero time of flight - future item

        // Detonate any shots that have reached their target
        void DetonateShots();
    }
}

void Engine::Load(std::vector<std::unique_ptr<Crobots::IRobot>>&& robots, Crobots::Arena arena)
{
    m_robots = std::move(robots);
    m_arena = arena;

    PlaceRobots();
}

void Engine::PlaceRobots()
{
    std::srand(std::time({}));
    for (std::unique_ptr<Crobots::IRobot>& robot : m_robots) {
        // Start each robot at a random spot in the arena.
        robot->m_locX = BoundedRand(m_arena.GetX());
        robot->m_locY = BoundedRand(m_arena.GetY());
    }
}

uint32_t Engine::BoundedRand(uint32_t range)
{
    for (uint32_t x, r;;)
        if (x = rand(), r = x % range, x - r <= -range)
            return r;
}

void Engine::MoveRobots()
{
}

void Engine::AccelRobots()
{
}

void Engine::MoveShotsInFlight()
{
}

void Engine::DetonateShots()
{
}

}
