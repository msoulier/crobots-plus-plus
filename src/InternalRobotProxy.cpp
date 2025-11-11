#include "Crobots++/InternalRobotProxy.hpp"

#include "Engine.hpp"

namespace Crobots
{

InternalRobotProxy::InternalRobotProxy(uint32_t id, std::shared_ptr<Engine> engine)
: m_id(id)
, m_engine(engine)
{}

float InternalRobotProxy::GetArenaX()
{
    return m_engine->GetArena().GetX();
}

float InternalRobotProxy::GetArenaY()
{
    return m_engine->GetArena().GetY();
}

float InternalRobotProxy::ScanResult(uint32_t id, float degree, float resolution)
{
    return m_engine->ScanResult(id, degree, resolution);
}

uint32_t InternalRobotProxy::GetId() const
{
    return m_id;
}

void InternalRobotProxy::SetId(const uint32_t id)
{
    m_id = id;
}

}
