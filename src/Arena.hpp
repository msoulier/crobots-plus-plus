#pragma once

#include <cstdint>
#include <vector>

namespace Crobots
{

class Position
{
public:
    uint32_t m_locX;
    uint32_t m_locY;
    uint32_t m_robotid;
};

class Arena
{
public:
    Arena() = default;
    Arena(const Arena&) = default;
    const Arena& operator=(const Arena& other);
    Arena(uint32_t x, uint32_t y);
    void SetX(uint32_t x);
    void SetY(uint32_t y);
    uint32_t GetX();
    uint32_t GetY();
    void SetPosition(uint32_t robotid, uint32_t locX, uint32_t locY);
    const std::vector<Position>& GetPositions(void);

private:
    uint32_t m_x;
    uint32_t m_y;

    // Need to move robot positions into the Arena to allow double buffering of robot moves.
    // I don't want to store robot positions in multiple places, so this will take
    // some refactoring. 
    std::vector<Position> m_positions;

};

}
