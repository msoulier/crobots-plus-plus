#pragma once

#include <numbers>

namespace Crobots
{

class InternalRobotProxy
{
public:
    InternalRobotProxy(uint32_t id);

    float GetArenaX();
    float GetArenaY();

private:
    uint32_t m_id;
};

}
