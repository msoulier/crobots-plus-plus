#pragma once

#include <vector>
#include <memory>

#include <Crobots++/IRobot.hpp>

#include "Arena.hpp"

namespace Crobots
{

class Engine
{
public:
    Engine() = default;
    Engine(const Engine&) = delete;
    const Engine& operator=(const Engine&) = delete;

    void Load(std::vector<std::unique_ptr<Crobots::IRobot>>&& robots, Arena arena);
    void Tick();
    std::vector<uint32_t> ScanResult(uint32_t degree, uint32_t resolution) const;

private:
    std::vector<std::unique_ptr<Crobots::IRobot>> m_robots;
    Crobots::Arena m_arena;

    // Initial random placement of the robots after loading.
    void PlaceRobots();
    void MoveRobots();
    void AccelRobots();
    void MoveShotsInFlight();
    void DetonateShots();
};

}
