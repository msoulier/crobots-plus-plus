#include "Loader.hpp"

namespace Crobots {

bool Loader::Load(std::string path)
{
    // FIXME
    return true;
}

std::vector<std::unique_ptr<Crobots::IRobot>>&& Loader::GetRobots()
{
    return std::move(m_robots);
}

}
