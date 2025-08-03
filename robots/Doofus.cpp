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
    }
};

CROBOTS_ENTRYPOINT IRobot* GetRobot()
{
    return new Doofus();
}
