#pragma once

#include <vector>
#include <memory>
#include <string_view>

#include <Crobots++/IRobot.hpp>

namespace Crobots {

class Loader
{
public:
    Loader() = default;

    /*
     * Note: The user should provide robot names. Then based on the platform
     * and a configured robots directory, we can find the appropriate file
     * to load based on the platform and its naming convention for shared libraries.
     */
    bool Load(const std::string& name, uint32_t id);
    std::vector<std::shared_ptr<Crobots::IRobot>>&& GetRobots();

private:
    std::vector<std::shared_ptr<Crobots::IRobot>> m_robots;
};

}
