#include <cstdlib>
#include <ctime>
#include <vector>
#include <cassert>
#include <numbers>

#include "Arena.hpp"
#include "Engine.hpp"

namespace Crobots
{

void Engine::Tick()
{
    //CROBOTS_LOG("Crobots::Engine::Tick()");
    for (std::unique_ptr<Crobots::IRobot>& robot : m_robots)
    {
        // Reset any internal tick counters.
        robot->UpdateTickCounters();
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

    // Set this engine as the static engine for IRobot
    IRobot::SetEngine(this);

    PlaceRobots();
}

uint32_t Engine::ScanResult(uint32_t robot_id, uint32_t direction, uint32_t resolution) const
{
    uint32_t result = 0;

    // Minimum resolution is 10.
    if (resolution < 10) {
        resolution = 10;
    }

    uint32_t fromX = m_robots[robot_id]->LocX();
    uint32_t fromY = m_robots[robot_id]->LocY();

    for (int i = 0; i < m_robots.size(); ++i) 
    {
        // Skip ourselves.
        if (i == robot_id) {
            continue;
        }
        uint32_t toX = m_robots[i]->LocX();
        uint32_t toY = m_robots[i]->LocY();
        // Simplify the math. Shift the grid until we are at the origin.
        toX -= fromX;
        toY -= fromY;
        // Now convert the "to" robot to polar coordinates.
        // https://www.mathsisfun.com/polar-cartesian-coordinates.html
        uint32_t radius = sqrt( toX*toX + toY*toY );
        uint32_t radians = atan( toY / toX );
        uint32_t degrees = radians * ( 180 / std::numbers::pi_v<float> );
        // Adjust for quadrant.
        if ((toX >= 0) && (toY >= 0))
        {
            // quadrant 1
            // nothing to do
        }
        else if ((toX < 0) && (toY >= 0))
        {
            // quadrant 2
            degrees += 180;
        }
        else if ((toX < 0) && (toY < 0))
        {
            // quadrant 3
            degrees += 180;
        }
        else
        {
            // quadrant 4
            degrees += 360;
        }
        // Now, to get a hit off of this contact, the scan direction plus or minus half of the resolution
        // must pass over the bearing.
        if (((direction - (resolution / 2)) <= degrees) || ((direction + (resolution / 2)) >= degrees))
        {
            // we have a hit we only return the closest one
            if (result == 0) {
                result = radius;
            }
            else if ((result > 0) && (result < radius))
            {
                result = radius;
            }
        }
        return result;
    }

}

void Engine::PlaceRobots()
{
    std::srand(std::time(nullptr));
    for (std::unique_ptr<Crobots::IRobot>& robot : m_robots)
    {
        // Start each robot at a random spot in the arena.
        robot->m_locX = IRobot::BoundedRand(m_arena.GetX());
        robot->m_locY = IRobot::BoundedRand(m_arena.GetY());
    }
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
