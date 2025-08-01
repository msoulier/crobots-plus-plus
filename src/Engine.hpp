#pragma once

#include <vector>
#include <Crobots++/IRobot.hpp>

#include "Arena.hpp"

namespace Crobots
{

class Engine
{
public:
    Engine();
    Engine(const Engine&) = delete;
    const Engine& operator=(const Engine&) = delete;
    ~Engine();

    void Load(std::vector<Crobots::IRobot> robots, Arena arena);
    void Tick();

private:
    std::vector<Crobots::IRobot> m_robots;
    Crobots::Arena m_arena;
};

}
