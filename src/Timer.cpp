#include <SDL3/SDL.h>

#include <cstdint>

#include "Timer.hpp"

namespace Crobots
{

Timer::Timer(float delay)
    : m_delay{delay}
    , m_time1{0}
    , m_time2{0}
    , m_deltaTime{0}
    , m_elapsedTime{0}
{
    Tick();
}

void Timer::Tick()
{
    if (m_elapsedTime >= m_delay)
    {
        m_elapsedTime -= m_delay;
    }
    m_time2 = SDL_GetTicksNS();
    m_deltaTime = (m_time2 - m_time1) / 1e6;
    m_time1 = m_time2;
    m_elapsedTime += m_deltaTime;
}

float Timer::GetDeltaTimeMS() const
{
    return m_deltaTime;
}

bool Timer::ShouldTick() const
{
    return m_elapsedTime >= m_delay;
}

}
