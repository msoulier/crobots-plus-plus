#include <Crobots++/IRobot.hpp>
#include <cassert>
#include <cmath>
#include <ctime>
#include <numbers>

#include "Crobots++/Log.hpp"
#include "Engine.hpp"

namespace Crobots {

// Static member. Populated in SetEngine().
Engine* IRobot::m_engine = nullptr;

// API methods - Usable by any Robot - ie. protected
//--------------------------------------------------

IRobot::IRobot()
{
    CROBOTS_LOG("IRobot ctor()");
    m_currentX = 0.0;
    m_currentY = 0.0;
    m_nextX = 0.0;
    m_nextY = 0.0;
    m_desiredSpeed = 0;
    m_speed = 0;
    m_desiredFacing = 0;
    m_facing = 0;
    m_damage = 0;
    m_cannonTimeUntilReload = 0;
    // This will need to eventually use a unique robot profile, but for now
    // everyone gets the same attributes.
    m_rounds = 65535; // TODO: use std::numeric_limits<uint16_t>::max() or UINT16_MAX
    m_acceleration = 1;
    m_braking = 5;
    m_turnRate = 5;
    m_cannonType = CannonType::Standard;
    m_cannonReloadTime = 3;
    // For now everyone has the same scanner.
    m_ticksPerScan = 10;
    m_scanCountDown = 0;
}

float IRobot::LocX()
{
    assert( m_currentX > 0 );
    return std::round(m_currentX);
}

float IRobot::LocY()
{
    assert( m_currentY > 0 );
    return std::round(m_currentY);
}

uint32_t IRobot::GetId() const
{
    return m_id;
}

void IRobot::SetId(uint32_t id)
{
    m_id = id;
}

float IRobot::GetX() const
{
    return m_currentX;
}

float IRobot::GetY() const
{
    return m_currentY;
}

uint32_t IRobot::Rand(uint32_t limit)
{
    return BoundedRand(limit);
}

uint32_t IRobot::Damage()
{
    return m_damage;
}

float IRobot::Facing()
{
    return m_facing;
}

float IRobot::Speed()
{
    return m_speed;
}

void IRobot::Drive(float degree, float speed)
{
    CROBOTS_LOG("Drive: degree = {}, speed = {}", degree, speed);
    if (degree < 0)
    {
        degree = 0.0;
    }
    else if (degree > 360)
    {
        degree = 0.0;
    }
    // TODO: use std::clamp
    if (speed < 0) {
        speed = 0;
    } else if (speed > 100) {
        speed = 100;
    }
    m_desiredFacing = degree;
    m_desiredSpeed = speed;
}

float IRobot::Scan(float degree, float resolution)
{
    if (m_scanCountDown > 0)
    {
        m_scanCountDown--;
        return -1;
    }
    m_scanCountDown = m_ticksPerScan;
    // We need to determine the bearing of each other robot to this one.
    // Once we have the bearing, based on 0 degrees to the right, and increasing counter-clockwise
    // to complete the circle, we can determine if the scan will ping off of one or more of them.
    assert( m_engine != nullptr );
    return m_engine->ScanResult(GetId(), degree, resolution);
}

bool IRobot::Cannon(float degree, float range)
{
    return RegisterShot(m_cannonType, degree, range);
}

// Note - The mathematical functions are not required due to the C++ standard library.
// https://cppreference.com/w/cpp/numeric/math.html

//--------------------------------------------------

// Private methods that we want accessible indirectly but no direct access by Robots
//----------------------------------------------------------------------------------
bool IRobot::RegisterShot(CannonType weapon, float degree, float range)
{
    // FIXME
    // Note, we do not care if a robot calls Cannon(), which calls this method, multiple times
    // in a Tick. Only the final shot registration will be acted upon.
    // if the cannon can be fired right now...
    m_cannonShotDegree = degree;
    m_cannonShotRange = range;
    m_cannotShotRegistered = true;
    return true;
}

uint32_t IRobot::BoundedRand(uint32_t range)
{
    assert( range > 0 );
    /* TODO: ideally we don't use rand() but instead rely on the random generators
    in <random>. that way we can improve reproducibility since there's no guarantees
    on the algorithm that rand() uses. */
    /* long term, we probably we a class that wraps <random> and makes sure to use
    the same seed so that the robots and engine are somewhat reproducible.

    e.g.

    // seed is some command line argument
    // name is the IRobot name or some reserved engine name
    RandomEngine(size_t seed, const std::string_view& name)
    {
        size_t real_seed = seed ^ std::hash<std::string_view>{}(name);
    }

    or

    RandomEngine(const std::unique_ptr<IRobot>& robot)
    {
        size_t some_global_seed = 0xD3ADB33F;
        size_t real_seed = some_global_seed ^ std::hash<std::string>{}(robot->GetName());
    }

    */
    uint32_t result = rand();
    return std::max(result % range, 1u);
}

void IRobot::UpdateTickCounters()
{
    m_cannotShotRegistered = false;
    // Manage cannon reload time.
    if (m_cannonTimeUntilReload > 0) {
        m_cannonTimeUntilReload--;
    }
}
//----------------------------------------------------------------------------------

// Static method
void IRobot::SetEngine(Engine* handle)
{
    m_engine = handle;
}

void IRobot::AccelRobot()
{
    if (IsDead())
    {
        m_speed = 0;
        return;
    }
    // Manage speed increase/decrease.
    if (m_speed != m_desiredSpeed)
    {
        if (m_desiredSpeed > m_speed)
        {
            m_speed += m_acceleration;
            if (m_speed > m_desiredSpeed)
            {
                m_speed = m_desiredSpeed;
            }
        }
        else
        {
            m_speed -= m_braking;
            if (m_speed < m_desiredSpeed)
            {
                m_speed = m_desiredSpeed;
            }
        }
        CROBOTS_LOG("speed is now {}", m_speed);
    }
    // Manage facing changes.
    if (m_desiredFacing != m_facing)
    {
        // Turn left or right?
        float left_distance = m_desiredFacing - m_facing;
        float right_distance = m_facing + ( 360 - m_desiredFacing );
        if (left_distance < right_distance)
        {
            m_facing += m_turnRate;
            if (m_facing > m_desiredFacing)
            {
                m_facing = m_desiredFacing;
            }
        }
        else
        {
            m_facing -= m_turnRate;
            if (m_facing < m_desiredFacing)
            {
                m_facing = m_desiredFacing;
            }
        }
    }
}

void IRobot::MoveRobot()
{
    if (IsDead())
    {
        return;
    }
    float arenaX = m_engine->GetArena().GetX();
    float arenaY = m_engine->GetArena().GetY();
    assert( arenaX > 0 );
    assert( arenaY > 0 );

    float radians = ToRadians(m_facing);
    float myspeed = GetActualSpeed();
    float x = myspeed * std::cos(radians);
    float y = myspeed * std::sin(radians);
    m_nextX = m_currentX + x;
    m_nextY = m_currentY + y;
    CROBOTS_LOG("speed is {}, x next {}, y next {}", myspeed, m_nextX, m_nextY);
    // Boundary check.
    if (m_nextX > arenaX)
    {
        m_nextX = arenaX;
        HitTheWall();
    }
    else if (m_nextX < 1)
    {
        m_nextX = 1;
        HitTheWall();
    }
    if (m_nextY > arenaY)
    {
        m_nextY = arenaY;
        HitTheWall();
    }
    else if (m_nextY < 1)
    {
        m_nextY = 1;
        HitTheWall();
    }
    assert( m_nextX > 0 );
    assert( m_nextY > 0 );
}

void IRobot::HitTheWall()
{
    m_damage += 5;
}

bool IRobot::IsDead()
{
    return m_damage >= 100;
}

float IRobot::ToDegrees(float radians)
{
    return radians * ( 180 / std::numbers::pi_v<float> );
}

float IRobot::ToRadians(float degrees)
{
    return ( degrees * std::numbers::pi_v<float> ) / 180;
}

float IRobot::GetActualSpeed()
{
    // speed is a percentage - FIXME: base this on the frame rate
    return m_speed / 200.0;
}

float IRobot::GetArenaX()
{
    return m_engine->GetArena().GetX();
}

float IRobot::GetArenaY()
{
    return m_engine->GetArena().GetY();
}

}
