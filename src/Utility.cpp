#include "Utility.hpp"
#include "Engine.hpp"

namespace Crobots {

uint32_t Utility::BoundedRand(uint32_t range)
{
    /* TODO: ideally we don't use rand() but instead rely on the random generators
    in <random>. that way we can improve reproducibility since there's no guarantees
    on the algorithm that rand() uses. */
    /* long term, we probably we a class that wraps <random> and makes sure to use
    the same seed so that the robots and engine are somewhat reproducible.

    e.g.

    // seed is some command line argument
    // name is the IRobot name or some reserved engine name
    RandomEngine(size_t seed, const std::string_view& name)
    {
        size_t real_seed = seed ^ std::hash<std::string_view>{}(name);
    }

    or

    RandomEngine(const std::unique_ptr<IRobot>& robot)
    {
        size_t some_global_seed = 0xD3ADB33F;
        size_t real_seed = some_global_seed ^ std::hash<std::string>{}(robot->GetName());
    }
    
    */
    for (uint32_t x, r;;)
        if (x = rand(), r = x % range, x - r <= -range)
            return r;
}

}
