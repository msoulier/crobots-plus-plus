#pragma once

#include <vector>
#include <memory>

#include <Crobots++/IRobot.hpp>

#include "Arena.hpp"
#include "Shot.hpp"

// Lets talk about velocity.
// I am modeling the arena dimensions after meters, so 100x100 is 100m on each side,
// roughly the size of a football field. We do not want the robots to cross it too
// quickly, that would be no fun, so I am aiming for about 10 1s ticks to cross
// 100m. Therefore, 100% speed is 10 m/s.
// This may not be the case for all robots in the future.
// To accurately model this, we want the position of the robot to be precise, rounding
// to the artificial grid construct, but secretly keeping more accurate positioning.
// So position will be stored as floats internally but returned as integers, rounded.

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
    uint32_t ScanResult(uint32_t robot_id, uint32_t degree, uint32_t resolution) const;
    void AddShot(Shot shot);
    const Arena& GetArena() const;
    const std::vector<std::unique_ptr<Crobots::IRobot>>& GetRobots() const;

private:
    std::vector<std::unique_ptr<Crobots::IRobot>> m_robots;
    std::vector<Shot> m_shots;
    Arena m_arena;

    // Initial random placement of the robots after loading.
    void PlaceRobots();
    void AccelRobots();
    void AddShots();
    void MoveShotsInFlight();
    void DetonateShots();
    void UpdateArena();
    void GameOver();
};

}
