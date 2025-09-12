#include <cstdlib>
#include <cmath>
#include <ctime>
#include <vector>
#include <cassert>
#include <numbers>

#include "Crobots++/IRobot.hpp"
#include "Api.hpp"
#include "Arena.hpp"
#include "Engine.hpp"

namespace Crobots
{

void Engine::Init(Crobots::Arena arena)
{
    m_arena = arena;
    // Static member initialization
    IRobot::SetEngine(this);
}

const Arena& Engine::GetArena() const
{
    return m_arena;
}

const std::vector<std::shared_ptr<Crobots::IRobot>>& Engine::GetRobots() const
{
    return m_robots;
}

void Engine::Tick()
{
    for (std::shared_ptr<Crobots::IRobot>& robot : m_robots)
    {
		CROBOTS_LOG("Engine looping on robot {}", robot->GetName());
        // Reset any internal tick counters.
        robot->UpdateTickCounters();
        // Run each robot through a tick.
        robot->Tick();
        // Update the position of each robot based on its velocity
        robot->MoveRobot();
        // Check for any loss of control (ie. skidding) - future item
        // Update the velocity (ie. speed and facing) of each robot
        robot->AccelRobot();
    }

    // Add any shots from robots firing now.
    AddShots();
    // Update the position of any shots in flight
    MoveShotsInFlight();

    // Fire any direct fire weapons that have zero time of flight - future item

    // Detonate any shots that have reached their target
    DetonateShots();

    // Update the arena.
    UpdateArena();
}

void Engine::Load(std::vector<std::shared_ptr<Crobots::IRobot>>&& robots)
{
	CROBOTS_LOG("Engine::Load: nrobots = {}", robots.size());
    m_robots = std::move(robots);

    PlaceRobots();
}

void Engine::AddShot(Shot shot)
{
    m_shots.push_back(shot);
}

void Engine::AddShots()
{
    // Check each robot for a pending shot.
    for (auto& robot : m_robots)
    {
        if (robot->IsDead())
        {
            continue;
        }
        if (robot->m_cannotShotRegistered)
        {
            Shot shot(robot->m_currentX,
                      robot->m_currentY,
                      robot->m_cannonShotDegree,
                      robot->m_cannonShotSpeed,
                      robot->m_cannonShotRange);
            AddShot(shot);
        }
    }
}

float Engine::ScanResult(uint32_t robot_id, uint32_t facing, uint32_t resolution) const
{
    float result = 0;

    // Minimum resolution is 10.
    if (resolution < 10) {
        resolution = 10;
    }

    // Need 2 or more robots to get a hit on scanning.
    if (m_robots.size() < 2)
    {
        return -1;
    }

    float myX = m_robots[robot_id]->LocX();
    float myY = m_robots[robot_id]->LocY();

    for (uint32_t i = 0; i < m_robots.size(); ++i)
    {
        // Skip ourselves.
        if (i == robot_id) {
            continue;
        }
        float theirX = m_robots[i]->LocX();
        float theirY = m_robots[i]->LocY();

        float diffX = theirX - myX;
        float diffY = theirY - myY;

        // Now convert the "to" robot to polar coordinates.
        // https://www.mathsisfun.com/polar-cartesian-coordinates.html
        float radius = sqrt( pow(diffX, 2) + pow(diffY, 2) );
        // FIXME: if radius > scanner_range, return 0
        float radians = atan2( diffY, diffX );
        float degrees = IRobot::ToDegrees(radians);
        // Adjust for quadrant. ?
        // Now, to get a hit off of this contact, the scan direction plus or minus half of the resolution
        // must pass over the bearing.
        if (((facing - (resolution / 2)) <= degrees) && ((facing + (resolution / 2)) >= degrees))
        {
            CROBOTS_LOG("Scanner contact: facing = {}, degrees = {}, radius = {}", facing, degrees, radius);
            // we have a hit we only return the closest one
            if (result == 0) {
                result = radius;
            }
            else if ((result > 0) && (result < radius))
            {
                result = radius;
            }
        }
    }
    return result;
}

void Engine::PlaceRobots()
{
    assert( m_arena.GetX() > 0 );
    assert( m_arena.GetY() > 0 );
    std::srand(std::time(nullptr));
    int count = 0;
    for (std::shared_ptr<Crobots::IRobot>& robot : m_robots)
    {
        // Start each robot at a random spot in the arena.
        robot->m_currentX = IRobot::BoundedRand(m_arena.GetX());
        robot->m_currentY = IRobot::BoundedRand(m_arena.GetY());
        CROBOTS_LOG("placing robot {} to initial location {}x{}",
            robot->GetName(), robot->m_currentX, robot->m_currentY);
    }
}

void Engine::MoveShotsInFlight()
{
    for (auto& shot : m_shots)
    {

    }
}

void Engine::DetonateShots()
{
}

void Engine::UpdateArena()
{
    uint32_t nRobotsAlive = 0;
    for (auto &robot : m_robots)
    {
        robot->m_currentX = robot->m_nextX;
        robot->m_currentY = robot->m_nextY;

        // Dead?
        if (robot->m_damage < 100)
        {
            nRobotsAlive++;
        }
    }
    // The threshold to end the game is 1 living robot, but for now,
    // for development, lets allow a single robot to run around.
    if (nRobotsAlive < 1)
    {
        GameOver();
    }
}

void Engine::GameOver()
{
    // We'll do more in the future. For now just shut down.
    CROBOTS_LOG("Game over");
    exit(0);
}

}
