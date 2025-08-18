#pragma once

#include <cstdint>
#include <vector>

namespace Crobots
{

class Arena
{
public:
    Arena() = default;
    Arena(const Arena&) = default;
    const Arena& operator=(const Arena& other);
    Arena(uint32_t x, uint32_t y);
    void SetX(uint32_t x);
    void SetY(uint32_t y);
    uint32_t GetX() const;
    uint32_t GetY() const;

private:
    uint32_t m_x;
    uint32_t m_y;
};

}
