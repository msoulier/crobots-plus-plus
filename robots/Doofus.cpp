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
        , m_last_damage{0}
        , m_last_scan_hit{false}
        , m_arenaMargin{10}
        , m_turn_countdown{0}
        , m_ticks_per_turn{100}
        , m_resolution{45.0f}
    {}
    // Note: API functions should not be used in the constructor, they are not ready until after
    // the constructor returns.

    uint32_t m_minimum_ticks_before_turn;
    float m_arenaX;
    float m_arenaY;
    uint32_t m_last_damage;
    bool m_last_scan_hit;
    float m_arenaMargin;
    uint32_t m_turn_countdown;
    uint32_t m_ticks_per_turn;
    float m_resolution;

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
        if (m_arenaX == 0)
        {
            m_arenaX = GetArenaX();
            m_arenaY = GetArenaY();
            assert( m_arenaX > 0 );
            assert( m_arenaY > 0 );
        }
        // What is my current position?
        float currentX = LocX();
        float currentY = LocY();
        float facing = Facing();
        float scan_direction = GetScanDir();
        uint32_t damage = Damage();

        CROBOTS_LOG("Doofus: x = {}, y = {}, facing = {}, last_scan_dir = {}, damage = {}",
            currentX, currentY, facing, scan_direction, damage);

        scan_direction += 10;

        float range = Scan(scan_direction, m_resolution);
        if (range > 0)
        {
            m_last_scan_hit = true;
            CROBOTS_LOG("===> Scan got a hit: direction {}, range {}", scan_direction, range);
            Drive(scan_direction, 100);
            // Tighten the resolution.
            m_resolution /= 2.0f;
            if (m_resolution < 10.0f)
            {
                m_resolution = 10.0f;
            }
            // And lets shoot at it if we can.
            bool shotfired = Cannon(scan_direction, range);
            if (shotfired)
            {
                CROBOTS_LOG("Bang!");
            }
            else
            {
                CROBOTS_LOG("Still reloading...");
            }
            return;
        }
        else
        {
            m_resolution = 45.0f;
            // FIXME: + or - ?
            scan_direction += 20;
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

CROBOTS_ENTRYPOINT IRobot* GetRobot(const std::shared_ptr<InternalRobotProxy>& proxy)
{
    return IRobot::Create<Doofus>(proxy);
}
