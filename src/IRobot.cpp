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
// Static members
std::random_device IRobot::rd;
std::mt19937 IRobot::gen(IRobot::rd());

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
    m_cannonReloadTime = 50;
    // For now everyone has the same scanner.
    m_ticksPerScan = 10;
    m_scanCountDown = 0;
    
    m_deathdata = {
        DamageType::Alive,
        {
            {0.0f, 0.0f, 0.0f, 0.0f}
        }
    };
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

float IRobot::GetFacing() const
{
    return m_facing;
}

float IRobot::GetDesiredFacing() const
{
    return m_desiredFacing;
}

uint32_t IRobot::Rand(uint32_t limit)
{
    return BoundedRand(limit);
}

uint32_t IRobot::Damage()
{
    return m_damage;
}

struct DeathData IRobot::GetDeathData() const
{
    return m_deathdata;
}

void IRobot::SetDeathData(struct DeathData deathdata)
{
    m_deathdata = deathdata;
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
    if (m_cannonTimeUntilReload > 0)
    {
        m_cannonTimeUntilReload--;
        return false;
    }
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
    std::uniform_int_distribution<int> dist(1, range);
    return dist(gen);
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
        CROBOTS_LOG("desired facing is not our facing: {} vs {}", m_desiredFacing, m_facing);
        // Turn left or right?
        float diff = 0.0f;
        if (m_desiredFacing > m_facing)
        {
            diff = m_desiredFacing - m_facing;
            if (diff > 180.0f)
            {
                // turn right
                m_facing -= m_turnRate;
                CROBOTS_LOG("right turn");
            }
            else
            {
                // turn left
                m_facing += m_turnRate;
                CROBOTS_LOG("left turn");
            }
        }
        else
        {
            diff = m_facing - m_desiredFacing;
            if (diff > 180.0f)
            {
                // turn left
                m_facing += m_turnRate;
                CROBOTS_LOG("left turn");
            }
            else
            {
                // turn right
                m_facing -= m_turnRate;
                CROBOTS_LOG("right turn");
            }
        }
        m_facing = Mod360(m_facing);
        CROBOTS_LOG("post mod360: {}", m_facing);
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
    if (m_damage >= 100)
    {
        struct DeathData ddata = {
            DamageType::HitWall,
            {
                { GetX(), GetY(), 100, GetFacing() }
            }
        };
        SetDeathData(ddata);
    }
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

float IRobot::Mod360(float number)
{
    float result = fmod(number, 360.0f);
    if (result < 0)
    {
        result += 360.0f;
    }
    return result;
}

}
