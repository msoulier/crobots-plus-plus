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
        , m_arenaMargin{10}
        , m_turn_countdown{0}
        , m_ticks_per_turn{100}
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
    float m_arenaMargin;
    uint32_t m_turn_countdown;
    uint32_t m_ticks_per_turn;

    std::string_view GetName() const override
    {
        return "Doofus";
    }

    bool NearWall(float x, float y)
    {
        if (NearTopWall(y) || NearBottomWall(y) ||
            NearRightWall(x) || NearLeftWall(x))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool NearTopWall(float y)
    {
        return y > (m_arenaY - m_arenaMargin);
    }

    bool NearBottomWall(float y)
    {
        return y < m_arenaMargin;
    }

    bool NearRightWall(float x)
    {
        return x > (m_arenaX - m_arenaMargin);
    }

    bool NearLeftWall(float x)
    {
        return x < m_arenaMargin;
    }

    void Tick() override
    {
        // What is my current position?
        float currentX = LocX();
        float currentY = LocY();
        float facing = Facing();
        float scan_direction = 0;
        uint32_t damage = Damage();

        CROBOTS_LOG("Doofus: x = {}, y = {}, facing = {}, last_scan_dir = {}, damage = {}",
            currentX, currentY, facing, m_last_scan_dir, damage);

        if (m_last_scan_dir == 720)
        {
            m_last_scan_dir = facing;
            scan_direction = facing;
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

        float range = Scan(scan_direction, 45);
        if (range > 0)
        {
            CROBOTS_LOG("===> Scan got a hit: direction {}, range {}", scan_direction, range);
        }

        if (NearWall(currentX, currentY))
        {
            CROBOTS_LOG("near the wall - m_turn_countdown is {}", m_turn_countdown);
        }
        if (m_last_damage != damage)
        {
            CROBOTS_LOG("taking damage, turn now");
            m_last_damage = damage;
            facing -= 45;
            facing = Mod360(facing);
            Drive(facing, 50);
            m_turn_countdown = m_ticks_per_turn;
        }
        else if ((m_turn_countdown == 0) && NearWall(currentX, currentY))
        {
            CROBOTS_LOG("ready to turn, and near the wall");
            facing -= 90;
            facing = Mod360(facing);
            Drive(facing, 50);
            m_turn_countdown = m_ticks_per_turn;
        }
        else
        {
            if (m_turn_countdown > 0)
            {
                m_turn_countdown--;
            }
            uint32_t speed = 100;
            if (GetDesiredFacing() != GetFacing())
            {
                // We're still turning.
                speed = 50;
            }
            Drive(GetDesiredFacing(), speed);
        }
    }
};

CROBOTS_ENTRYPOINT IRobot* GetRobot()
{
    return new Doofus();
}
