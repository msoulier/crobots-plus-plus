#pragma once

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
};

}
