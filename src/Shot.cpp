#include "Shot.hpp"

namespace Crobots {

Shot::Shot(float initialX,
           float initialY,
           uint32_t facing,
           uint32_t speed,
           uint32_t range)
    : m_currentX(initialX)
    , m_currentY(initialY)
    , m_facing(facing)
    , m_speed(speed)
    , m_range(range)
{}

}
