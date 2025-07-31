#pragma once

#include <cstdint>

namespace Crobots
{

class Arena
{
public:
    Arena(uint32_t x, uint32_t y);

private:
    uint32_t m_x;
    uint32_t m_y;

};

}
