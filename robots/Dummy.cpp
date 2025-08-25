#include <iostream>

#include <Crobots++/Crobots++.hpp>

using namespace Crobots;

class Dummy : public IRobot
{
public:
    Dummy() = default;

    std::string_view GetName() const override
    {
        return "Dummy";
    }

    void Tick() override
    {
        CROBOTS_LOG("Dummy.Tick");
        // What is my current position?
        uint32_t currentX = LocX();
        uint32_t currentY = LocY();
        CROBOTS_LOG("x = {}, y = {}, damage = {}", currentX, currentY, Damage());

        uint32_t new_facing = Facing();

        if (currentX > 95)
        {
            if (currentY < 50)
            {
                new_facing = 190;
            }
            else
            {
                new_facing = 170;
            }
        }
        else if (currentX < 5)
        {
            if (currentY < 50)
            {
                new_facing = 350;
            }
            else
            {
                new_facing = 10;
            }
        }
        //new_facing = Rand(360);
        Drive(new_facing, 100);
        CROBOTS_LOG("driving at facing {} at max speed", Facing());
    }
};

CROBOTS_ENTRYPOINT IRobot* GetRobot()
{
    return new Dummy();
}
