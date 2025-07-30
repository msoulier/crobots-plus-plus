#pragma once

#include <cstdint>

namespace Crobots
{

// Times in nanoseconds (except delay), and the engine wants to work in ms.
class Timer
{
public:
    // delay is time in ms
    Timer() = default;
    Timer(const Timer&) = default;
    Timer& operator=(const Timer&) = default;
    Timer(float delay);
    void Tick();
    float GetDeltaTimeMS() const;
    bool ShouldTick() const;
private:
    float m_delay;
    uint64_t m_time1;
    uint64_t m_time2;
    float m_deltaTime;
    float m_elapsedTime;
};

}
