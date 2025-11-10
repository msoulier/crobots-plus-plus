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
    uint32_t GetId() const;
    void SetId(const uint32_t id);

private:
    // This id should be a simple integer uniquely identifying the robot based on the order
    // in which it was loaded.
    uint32_t m_id;
    // FIXME: check for circular reference
    std::shared_ptr<Engine> m_engine;
};

}
