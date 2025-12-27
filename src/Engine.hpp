#pragma once

#include <Crobots++/IRobot.hpp>
#include <vector>
#include <memory>

#include "Api.hpp"
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

class Position
{
public:
    Position(float x, float y);
    float GetX();
    float GetY();
    void SetX(float x);
    void SetY(float y);
private:
    float m_x;
    float m_y;
};

class Engine
{
public:
    Engine() = default;
    Engine(const Engine&) = delete;
    const Engine& operator=(const Engine&) = delete;

    void Init(Arena arena, bool debug, bool damage, bool pause_on_scan);
    void Load(std::vector<std::shared_ptr<IRobot>>&& robots);
    void Tick();
    float ScanResult(uint32_t robot_id, float degree, float resolution) const;
    void AddShot(Shot shot);
    const Arena& GetArena() const;
    const std::vector<std::shared_ptr<IRobot>>& GetRobots() const;
    const std::vector<Shot>& GetShots() const;
    bool DebugEnabled() const;

    // This method is a utility method for computing a position a provided
    // distance along the current path of an object.
    static Position GetPositionAhead(float x, float y, float facing, float distance);

private:
    std::vector<std::shared_ptr<IRobot>> m_robots;
    std::vector<Shot> m_shots;
    Arena m_arena;
    bool m_debug;
    bool m_damage;
    bool m_pause_on_scan;

    // Initial random placement of the robots after loading.
    void PlaceRobots();
    void AddShots();
    void MoveShotsInFlight();
    void DetonateShots();
    void UpdateArena();
    void GameOver();

};

}
