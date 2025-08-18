#include <cstdlib>
#include <cmath>
#include <ctime>
#include <vector>
#include <cassert>
#include <numbers>

#include "Api.hpp"
#include "Arena.hpp"
#include "Engine.hpp"

namespace Crobots
{

const Arena& Engine::GetArena()
{
    return m_arena;
}

void Engine::Tick()
{
    CROBOTS_LOG("Engine.Tick on {} robots", m_robots.size());
    for (std::unique_ptr<Crobots::IRobot>& robot : m_robots)
    {
		CROBOTS_LOG("Engine looping on robot {}", robot->GetName());
        // Reset any internal tick counters.
        robot->UpdateTickCounters();
        // Run each robot through a tick.
        robot->Tick();
        // Update the position of each robot based on its velocity
        robot->MoveRobot();
        // Update the velocity (ie. speed and facing) of each robot
        AccelRobots();
        // Check for any loss of control (ie. skidding) - future item

        AddShots();
        // Update the position of any shots in flight
        MoveShotsInFlight();

        // Fire any direct fire weapons that have zero time of flight - future item

        // Detonate any shots that have reached their target
        DetonateShots();

        // Update the arena.
        UpdateArena();
    }
}

void Engine::Load(std::vector<std::unique_ptr<Crobots::IRobot>>&& robots, Crobots::Arena arena)
{
	CROBOTS_LOG("Engine::Load: nrobots = {}, arena = {}x{}", robots.size(), arena.GetX(), arena.GetY());
    m_robots = std::move(robots);
    m_arena = arena;
    CROBOTS_LOG("my arena is {} x {}", m_arena.GetX(), m_arena.GetY());

    // Set this engine as the static engine for IRobot
    IRobot::SetEngine(this);

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

uint32_t Engine::ScanResult(uint32_t robot_id, uint32_t direction, uint32_t resolution) const
{
    uint32_t result = 0;

    // Minimum resolution is 10.
    if (resolution < 10) {
        resolution = 10;
    }

    uint32_t fromX = m_robots[robot_id]->LocX();
    uint32_t fromY = m_robots[robot_id]->LocY();

    for (uint32_t i = 0; i < m_robots.size(); ++i)
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
        uint32_t degrees = IRobot::ToDegrees(radians);
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
    }
    return result;
}

void Engine::PlaceRobots()
{
    assert( m_arena.GetX() > 0 );
    assert( m_arena.GetY() > 0 );
    std::srand(std::time(nullptr));
    for (std::unique_ptr<Crobots::IRobot>& robot : m_robots)
    {
        // Start each robot at a random spot in the arena.
        robot->m_currentX = IRobot::BoundedRand(m_arena.GetX());
        robot->m_currentY = IRobot::BoundedRand(m_arena.GetY());
    }
}

void Engine::AccelRobots()
{
    for (const auto& robot : m_robots) {
        // Manage speed
        if (robot->m_desiredSpeed != robot->m_speed)
        {
            if (robot->m_desiredSpeed < robot->m_speed)
            {
                if ((robot->m_speed - robot->m_desiredSpeed) <= robot->m_braking)
                {
                    robot->m_speed = robot->m_desiredSpeed;
                }
            }
            else
            {
                robot->m_speed += robot->m_acceleration;
                if (robot->m_speed > robot->m_desiredSpeed)
                {
                    robot->m_speed = robot->m_desiredSpeed;
                }
            }
        }
        // Manage facing.
        if (robot->m_desiredFacing != robot->m_facing)
        {
            // Turn left or right?
            uint32_t left_distance = robot->m_desiredFacing - robot->m_facing;
            uint32_t right_distance = robot->m_facing + ( 360 - robot->m_desiredFacing );
            if (left_distance < right_distance)
            {
                robot->m_facing += robot->m_turnRate;
                if (robot->m_facing > robot->m_desiredFacing)
                {
                    robot->m_facing = robot->m_desiredFacing;
                }
            }
            else
            {
                robot->m_facing -= robot->m_turnRate;
                if (robot->m_facing < robot->m_desiredFacing)
                {
                    robot->m_facing = robot->m_desiredFacing;
                }
            }
        }
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
    for (auto &robot : m_robots)
    {
        robot->m_currentX = robot->m_nextX;
        robot->m_currentY = robot->m_nextY;
        // FIXME: turn rate
        robot->m_facing = robot->m_desiredFacing;
    }
}

}
