#include <iostream>
#include <cassert>

#include <Crobots++/Crobots++.hpp>

using namespace Crobots;

class Doofus : public IRobot
{
public:
    Doofus()
        : m_minimum_ticks_before_turn{20}
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
    uint32_t m_arenaX;
    uint32_t m_arenaY;
    uint32_t m_last_scan_dir;
    uint32_t m_last_damage;
    bool m_last_scan_hit;

    std::string_view GetName() const override
    {
        return "Doofus";
    }

    bool NearTopWall(uint32_t y)
    {
        return y > (m_arenaY - 5);
    }

    bool NearBottomWall(uint32_t y)
    {
        return y < 5;
    }

    bool NearRightWall(uint32_t x)
    {
        return x > (m_arenaX - 5);
    }

    bool NearLeftWall(uint32_t x)
    {
        return x < 5;
    }

    bool NearWall(uint32_t x, uint32_t y)
    {
        if (NearTopWall(y))
        {
            CROBOTS_LOG("I'm near the top wall");
            return true;
        }
        else if (NearBottomWall(y))
        {
            CROBOTS_LOG("I am near the bottom wall");
            return true;
        }
        else if (NearLeftWall(x))
        {
            CROBOTS_LOG("I am near the left wall");
            return true;
        }
        else if (NearRightWall(x))
        {
            CROBOTS_LOG("I am near the right wall");
            return true;
        }
        else
        {
            return false;
        }
    }

    void Tick() override
    {
        CROBOTS_LOG("Doofus.Tick");
        // What is my current position?
        uint32_t currentX = LocX();
        uint32_t currentY = LocY();
        CROBOTS_LOG("x = {}, y = {}, damage = {}", currentX, currentY, Damage());

        uint32_t new_facing = Facing();
        uint32_t scan_direction = 0;
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

        bool want_to_turn = false;
        bool taking_damage = false;
        uint32_t current_damage = Damage();
        if (m_last_damage != current_damage)
        {
            CROBOTS_LOG("We're taking damage!");
            taking_damage = true;
            m_last_damage = current_damage;
            want_to_turn = true;
        }

        if (NearWall(currentX, currentY))
        {
            CROBOTS_LOG("We want to turn");
            want_to_turn = true;
        }

        uint32_t range = 0;
        if (range = Scan(scan_direction, 45) > 0)
        {
            CROBOTS_LOG("Scan turned up a hit! direction {}, range {}", scan_direction, range);
            // Got a hit. Drive that way.
            m_last_scan_hit = true;
            if (scan_direction != new_facing)
            {
                new_facing = scan_direction;
            }
        }
        else
        {
            CROBOTS_LOG("Nothing on the scan");
            m_last_scan_hit = false;
            if (want_to_turn)
            {
                if (m_minimum_ticks_before_turn > 0)
                {
                    m_minimum_ticks_before_turn--;
                }
                else
                {
                    m_minimum_ticks_before_turn = 20;
                    new_facing += 90;
                    new_facing %= 360;
                }
            }
        }

        Drive(new_facing, 100);
    }
};

CROBOTS_ENTRYPOINT IRobot* GetRobot()
{
    return new Doofus();
}
