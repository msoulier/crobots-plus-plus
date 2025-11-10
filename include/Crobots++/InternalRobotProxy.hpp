#pragma once

#include <numbers>

#include "Engine.hpp"

namespace Crobots
{

class InternalRobotProxy
{
public:
    InternalRobotProxy(uint32_t id, std::shared_ptr<Engine> engine);

    float GetArenaX();
    float GetArenaY();
    float ScanResult(uint32_t id, float degree, float resolution);

private:
    uint32_t m_id;
    std::shared_ptr<Engine> m_engine;
};

}
