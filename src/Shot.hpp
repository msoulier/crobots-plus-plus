#pragma once

#include <Crobots++/IRobot.hpp>

namespace Crobots {

// This class represents a shot in-flight, guided or unguided.
// Initially just an unguide cannon shot, but maybe more in the future.
class Shot {
public:
    Shot(float initialX,
         float initialY,
         uint32_t facing,
         uint32_t speed,
         uint32_t range);
    Shot(const Shot&) = default;
    const Shot& operator=(const Shot& other);

private:
    // Current X and Y location. To allow high resolution of movement, the coordinates that the
    // robot stores are multiplied by 100.
    float m_currentX;
    float m_currentY;
    // Post-move X and Y location.
    float m_nextX;
    float m_nextY;
    // Current speed
    uint32_t m_speed;
    // Facing we currently have.
    uint32_t m_facing;
    // Range of the shot. How long will it be in the air.
    // It gets decremented for every meter it moves.
    uint32_t m_range;
};

}
