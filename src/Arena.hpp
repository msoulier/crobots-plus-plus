#pragma once

#include <cstdint>

namespace Crobots
{

class Arena
{
public:
    Arena() = default;
    Arena(uint32_t x, uint32_t y);
    void SetX(uint32_t x);
    void SetY(uint32_t y);
    uint32_t GetX();
    uint32_t GetY();

private:
    uint32_t m_x;
    uint32_t m_y;

};

}
