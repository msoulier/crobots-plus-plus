#include "Utility.hpp"
#include "Engine.hpp"

namespace Crobots {

uint32_t Utility::BoundedRand(uint32_t range)
{
    for (uint32_t x, r;;)
        if (x = rand(), r = x % range, x - r <= -range)
            return r;
}

}
