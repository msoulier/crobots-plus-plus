#include <iostream>

#include <Crobots++/Crobots++.hpp>

using namespace Crobots;

class Doofus : public IRobot
{
public:
    Doofus() = default;

    std::string_view GetName() const override
    {
        return "Doofus";
    }

    void Tick() override
    {
        CROBOTS_LOG("Doofus.Tick");
        // What is my current position?
        uint32_t currentX = LocX();
        uint32_t currentY = LocY();
        CROBOTS_LOG("x = {}, y = {}, damage = {}", currentX, currentY, Damage());

        uint32_t new_facing = Facing();
        if ((new_facing != 270) && (new_facing != 90))
        {
            new_facing = 90;
        }

        if (currentY > 95)
        {
            new_facing = 270;;
        }
        else if (currentY < 5)
        {
            new_facing = 90;
        }
        //new_facing = Rand(360);
        Drive(new_facing, 100);
        CROBOTS_LOG("driving at facing {} at max speed", Facing());
    }
};

CROBOTS_ENTRYPOINT IRobot* GetRobot()
{
    return new Doofus();
}
