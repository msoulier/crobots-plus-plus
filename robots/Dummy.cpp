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
        if ((new_facing != 0) && (new_facing != 180))
        {
            new_facing = 0;
        }

        if (currentX > 95)
        {
            new_facing = 180;
        }
        else if (currentX < 5)
        {
            new_facing = 0;
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
