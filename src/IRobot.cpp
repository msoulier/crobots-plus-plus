#include <Crobots++/IRobot.hpp>

#include "Utility.hpp"

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
    return Utility::BoundedRand(limit);
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
