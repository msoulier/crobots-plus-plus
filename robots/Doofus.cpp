#include <Crobots++/Crobots++.hpp>

using namespace Crobots;

class Doofus : public IRobot
{
public:
    Doofus() = default;

    std::string GetName() const override
    {
        return "Doofus";
    }
};

CROBOTS_ENTRYPOINT IRobot* GetRobot()
{
    return new Doofus();
}