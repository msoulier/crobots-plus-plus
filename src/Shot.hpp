#pragma once

#include <Crobots++/IRobot.hpp>

namespace Crobots {

// This class represents a shot in-flight, guided or unguided.
// Initially just an unguide cannon shot, but maybe more in the future.
class Shot {
private:
    friend class Engine;
public:
    Shot(float initialX,
         float initialY,
         float facing,
         float speed,
         float range);
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
    float m_speed;
    // Facing we currently have.
    float m_facing;
    // Range of the shot. How long will it be in the air.
    // It gets decremented for every meter it moves.
    float m_range;
    // Remaining range until detonation.
    float m_remainingRange;
};

}
