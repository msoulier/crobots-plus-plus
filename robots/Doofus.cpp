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
        CROBOTS_LOG("x = {}, y = {}", currentX, currentY);

        uint32_t new_facing = Facing();
        if ((currentX > 90)
         || (currentY > 90)
         || (currentX < 10)
         || (currentY < 10))
        {
            new_facing = Rand(360);
        }
        Drive(new_facing, 100);
    }
};

CROBOTS_ENTRYPOINT IRobot* GetRobot()
{
    return new Doofus();
}
