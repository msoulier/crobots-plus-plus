#include <iostream>
#include <cassert>

#include <Crobots++/Crobots++.hpp>

using namespace Crobots;

class Doofus : public IRobot
{
public:
    Doofus()
        : m_minimum_ticks_before_turn{0}
        , m_arenaX{0}
        , m_arenaY{0}
        , m_last_scan_dir{720}
        , m_last_damage{0}
        , m_last_scan_hit{false}
    {
        m_arenaX = GetArenaX();
        m_arenaY = GetArenaY();
        assert( m_arenaX > 0 );
        assert( m_arenaY > 0 );
    }

    uint32_t m_minimum_ticks_before_turn;
    float m_arenaX;
    float m_arenaY;
    float m_last_scan_dir;
    uint32_t m_last_damage;
    bool m_last_scan_hit;
    float m_arenaMargin = 10;

    std::string_view GetName() const override
    {
        return "Doofus";
    }

    bool NearTopWall(uint32_t y)
    {
        return y > (m_arenaY - m_arenaMargin);
    }

    bool NearBottomWall(uint32_t y)
    {
        return y < m_arenaMargin;
    }

    bool NearRightWall(uint32_t x)
    {
        return x > (m_arenaX - m_arenaMargin);
    }

    bool NearLeftWall(uint32_t x)
    {
        return x < m_arenaMargin;
    }

    void Tick() override
    {
        // What is my current position?
        float currentX = LocX();
        float currentY = LocY();
        float new_facing = Facing();
        float scan_direction = 0;
        CROBOTS_LOG("Doofus: x = {}, y = {}, facing = {}, last_scan_dir = {}, damage = {}",
            currentX, currentY, new_facing, m_last_scan_dir, Damage());

        if (m_last_scan_dir == 720)
        {
            m_last_scan_dir = new_facing;
            scan_direction = new_facing;
        }
        else
        {
            if (m_last_scan_hit)
            {
                // FIXME: tighten the scan - track the target
                scan_direction = m_last_scan_dir;
            }
            else
            {
                // Keep rotating scan like a radar.
                scan_direction += 90;
            }
        }

        bool taking_damage = false;
        uint32_t current_damage = Damage();
        if (m_last_damage != current_damage)
        {
            CROBOTS_LOG("We're taking damage!");
            taking_damage = true;
            m_last_damage = current_damage;
        }

        if (NearTopWall(currentY))
        {
            CROBOTS_LOG("I'm near the top wall");
            new_facing = 270;
        }
        else if (NearBottomWall(currentY))
        {
            CROBOTS_LOG("I am near the bottom wall");
            new_facing = 90;
        }
        else if (NearLeftWall(currentX))
        {
            CROBOTS_LOG("I am near the left wall");
            new_facing = 0;
        }
        else if (NearRightWall(currentX))
        {
            CROBOTS_LOG("I am near the right wall");
            new_facing = 180;
        }

        float range = 0;
        if (range = Scan(scan_direction, 45) > 0)
        {
            CROBOTS_LOG("Scan turned up a hit! direction {}, range {}", scan_direction, range);
        }
        else
        {
            CROBOTS_LOG("Nothing on the scan");
            m_last_scan_hit = false;
        }

        if (m_minimum_ticks_before_turn > 0 && !taking_damage)
        {
            m_minimum_ticks_before_turn--;
        }
        else
        {
            m_minimum_ticks_before_turn = 20;
            Drive(new_facing, 100);
            CROBOTS_LOG("Driving full speed, heading {}", new_facing);
        }
    }
};

CROBOTS_ENTRYPOINT IRobot* GetRobot()
{
    return new Doofus();
}
