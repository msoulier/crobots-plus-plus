#pragma once

#include <string>

/*
 * e.g.
 *
 * #include <Crobots++/IRobot.hpp>
 *
 * using namespace Crobots;
 *
 * class Doofus : public IRobot
 * {
 *     Doofus() = default;
 * 
 *     std::string GetName() const override
 *     {
 *         return "Doofus";
 *     }
 * };
 * 
 * CROBOTS_ENTRYPOINT IRobot* GetRobot()
 * {
 *     return new Doofus();
 * }
 *
 */

#if defined(_WIN32) || defined(__CYGWIN__)
#define CROBOTS_ENTRYPOINT __declspec(dllexport)
#elif defined(__GNUC__) || defined(__clang__)
#define CROBOTS_ENTRYPOINT __attribute__((visibility("default")))
#else
#define CROBOTS_ENTRYPOINT
#endif

namespace Crobots
{

class IRobot
{
public:
    virtual ~IRobot() = default;
    virtual std::string GetName() const = 0;

};

}