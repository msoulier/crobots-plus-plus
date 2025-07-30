#pragma once

#include <SDL3/SDL.h>

#ifndef NDEBUG
#define CROBOTS_DEBUG_GROUP(...) ::Crobots::DebugGroup debugGroup(__VA_ARGS__, __func__)
#else
#define CROBOTS_DEBUG_GROUP(...)
#endif

#ifndef NDEBUG
namespace Crobots
{

struct DebugGroup
{
    DebugGroup(SDL_GPUCommandBuffer* commandBuffer, const char* name);
    ~DebugGroup();

    SDL_GPUCommandBuffer* m_commandBuffer;
};

}
#endif