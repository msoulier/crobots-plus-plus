#pragma once

#include <string>

namespace Crobots
{

class IRobot
{
public:
    virtual ~IRobot() = default;
    virtual std::string GetName() const = 0;

};

}