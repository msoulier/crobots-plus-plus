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
    {
        m_arenaX = GetArenaX();
        m_arenaY = GetArenaY();
        assert( m_arenaX > 0 );
        assert( m_arenaY > 0 );
    }

    uint32_t m_minimum_ticks_before_turn;
    uint32_t m_arenaX;
    uint32_t m_arenaY;

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
        if (NearTopWall(y) || NearBottomWall(y) || NearLeftWall(x) || NearRightWall(x))
        {
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

        if (NearWall(currentX, currentY))
        {
            CROBOTS_LOG("We are near a wall and need to turn, arena {}x{}", m_arenaX, m_arenaY);
            new_facing += 90;
            Drive(new_facing, 100);
            m_minimum_ticks_before_turn = 20;
        }
        else
        {
            // Scan in the direction of movement.
            if (Scan(new_facing, 45))
            {
                Drive(new_facing, 100);
                m_minimum_ticks_before_turn = 20;
            }
            else
            {
                m_minimum_ticks_before_turn--;
                if (m_minimum_ticks_before_turn == 0)
                {
                    new_facing -= 90;
                    new_facing %= 360;
                    Drive(new_facing, 100);
                }
            }
        }

        CROBOTS_LOG("driving at facing {} at max speed", Facing());
    }
};

CROBOTS_ENTRYPOINT IRobot* GetRobot()
{
    return new Doofus();
}
