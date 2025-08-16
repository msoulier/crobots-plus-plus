#include <iostream>

#include <Crobots++/Crobots++.hpp>
#include <Crobots++/ClientLog.hpp>

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
        ROBOTLOG("Doofus.Tick");
        // What is my current position?
        uint32_t currentX = LocX();
        uint32_t currentY = LocY();
        ROBOTLOG("x = {}, y = {}", currentX, currentY);
    }
};

CROBOTS_ENTRYPOINT IRobot* GetRobot()
{
    return new Doofus();
}
