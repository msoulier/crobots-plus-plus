#pragma once

#include <SDL3/SDL.h>

#ifndef NDEBUG
#define CROBOTS_DEBUG_GROUP(commandBuffer) Crobots::DebugGroup debugGroup(commandBuffer, __func__)
#else
#define CROBOTS_DEBUG_GROUP(commandBuffer)
#endif

#ifndef NDEBUG
namespace Crobots
{

struct DebugGroup
{
    DebugGroup(SDL_GPUCommandBuffer* commandBuffer, const char* name);
    ~DebugGroup();

    SDL_GPUCommandBuffer* commandBuffer;
};

}
#endif