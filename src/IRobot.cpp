#include <Crobots++/IRobot.hpp>

namespace Crobots {

// Static member. Populated in SetEngine().
Engine* IRobot::m_engine = nullptr;

// API methods - Usable by any Robot - ie. protected
//--------------------------------------------------

IRobot::IRobot()
{
    m_locX = 0;
    m_locY = 0;
    m_desiredSpeed = 0;
    m_speed = 0;
    m_desiredFacing = 0;
    m_facing = 0;
    m_damage = 0;
    // This will need to eventually use a unique robot profile, but for now
    // everyone gets the same attributes.
    m_rounds = 65535; // TODO: use std::numeric_limits<uint16_t>::max() or UINT16_MAX
    m_cstate = CannonState::Ready;
    m_acceleration = 100;
    m_braking = 100;
    m_turnRate = 90;
}

uint32_t IRobot::LocX()
{
    return m_locX;
}

uint32_t IRobot::LocY()
{
    return m_locY;
}

uint32_t IRobot::Rand(uint32_t limit)
{
    return BoundedRand(limit);
}

uint8_t IRobot::Damage()
{
    return m_damage;
}

uint8_t IRobot::Speed()
{
    return m_speed;
}

void IRobot::Drive(uint16_t degree, uint8_t speed)
{
    degree %= 360;
    // TODO: use std::clamp
    if (speed < 0) {
        speed = 0;
    } else if (speed > 100) {
        speed = 100;
    }
    m_desiredFacing = degree;
    m_desiredSpeed = speed;
}

uint32_t IRobot::Scan(uint16_t degree, uint16_t resolution)
{
    // FIXME
    return 0;
}

bool IRobot::Cannon(uint16_t degree, uint32_t range)
{
    // FIXME
    return false;
}

uint32_t IRobot::Sqrt(uint32_t number)
{
    // FIXME
    return 0;
}

uint32_t IRobot::Sin(uint32_t degree)
{
    // FIXME
    return 0;
}

uint32_t IRobot::Cos(uint32_t degree)
{
    // FIXME
    return 0;
}

uint32_t IRobot::Tan(uint32_t degree)
{
    // FIXME
    return 0;
}

uint32_t IRobot::Atan(uint32_t ratio)
{
    // FIXME
    return 0;
}

//--------------------------------------------------

// Private methods that we want accessible indirectly but no direct access by Robots
//----------------------------------------------------------------------------------
uint32_t IRobot::BoundedRand(uint32_t range)
{
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
    for (uint32_t x, r;;)
        if (x = rand(), r = x % range, x - r <= -range)
            return r;
}
//----------------------------------------------------------------------------------

// Static method
void IRobot::SetEngine(Engine* handle)
{
    m_engine = handle;
}

}
