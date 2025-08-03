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
    Utility utility;
    return utility.BoundedRand(limit);
}

}
