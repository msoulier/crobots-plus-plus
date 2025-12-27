#include "Shot.hpp"

namespace Crobots {

Shot::Shot(float initialX,
           float initialY,
           float facing,
           float speed,
           float range)
    : m_currentX{initialX}
    , m_currentY{initialY}
    , m_facing{facing}
    , m_speed{speed}
    , m_range{range}
    , m_remainingRange{0}
{}

float Shot::GetX() const
{
    return m_currentX;
}

float Shot::GetY() const
{
    return m_currentY;
}

void Shot::SetX(float x)
{
    m_currentX = x;
}

void Shot::SetY(float y)
{
    m_currentY = y;
}

float Shot::GetFacing() const
{
    return m_facing;
}

float Shot::GetSpeed() const
{
    return m_speed;
}

}
