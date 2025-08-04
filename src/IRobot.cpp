#include <Crobots++/IRobot.hpp>

namespace Crobots {

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

void IRobot::Init(IRobot* robot)
{
    robot->m_locX = 0;
    robot->m_locY = 0;
    robot->m_desiredSpeed = 0;
    robot->m_speed = 0;
    robot->m_desiredFacing = 0;
    robot->m_facing = 0;
    robot->damage = 0;
    // This will need to eventually use a unique robot profile, but for now
    // everyone gets the same attributes.
    robot->m_rounds = 65535;
    robot->m_cstate = CannonState::Ready;
    robot->acceleration = 100;
    robot->braking = 100;
    robot->turnRate = 90;
}

void IRobot::SetEngine(Engine* handle)
{
    m_engine = handle;
}

IRobot::Engine* m_engine;

}
