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

    bool Load(const std::string& path);
    std::vector<std::unique_ptr<Crobots::IRobot>>&& GetRobots();

private:
    std::vector<std::unique_ptr<Crobots::IRobot>> m_robots;
};

}
