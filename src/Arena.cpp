#include "Arena.hpp"

namespace Crobots
{

Arena::Arena(uint32_t x, uint32_t y)
    : m_x{x}
    , m_y{y} {}

const Arena& Arena::operator=(const Arena& other)
{
    if (this != &other)
    {
        this->m_x = other.m_x;
        this->m_y = other.m_y;
    }
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

uint32_t Arena::GetX() const
{
    return m_x;
}

uint32_t Arena::GetY() const
{
    return m_y;
}

}
