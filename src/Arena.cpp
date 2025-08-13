#include "Arena.hpp"

namespace Crobots
{

Arena::Arena(uint32_t x, uint32_t y)
    : m_x{x}
    , m_y{y} {}

const Arena& Arena::operator=(const Arena& other)
{
    return *this;
}

void Arena::SetX(uint32_t x)
{
    m_x = x;
}

void Arena::SetY(uint32_t y)
{
    m_y = y;
}

uint32_t Arena::GetX()
{
    return m_x;
}

uint32_t Arena::GetY()
{
    return m_y;
}

}
