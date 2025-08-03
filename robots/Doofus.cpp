#include <Crobots++/Crobots++.hpp>
#include <iostream>

using namespace Crobots;

class Doofus : public IRobot
{
public:
    Doofus() = default;

    std::string GetName() const override
    {
        return "Doofus";
    }

    void Tick() override
    {
        std::cout << "Doofus tick" << std::endl;
        // What is my current position?
        uint32_t currentX = LocX();
        uint32_t currentY = LocY();
    }
};

CROBOTS_ENTRYPOINT IRobot* GetRobot()
{
    return new Doofus();
}
