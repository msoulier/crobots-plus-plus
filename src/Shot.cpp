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

}
