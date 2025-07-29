#include <SDL3/SDL.h>

#include "DebugGroup.hpp"

#ifndef NDEBUG
namespace Crobots
{

DebugGroup::DebugGroup(SDL_GPUCommandBuffer* commandBuffer, const char* name) : commandBuffer{commandBuffer}
{
    SDL_PushGPUDebugGroup(commandBuffer, name);
}

DebugGroup::~DebugGroup()
{
    SDL_PopGPUDebugGroup(commandBuffer);
}

}
#endif