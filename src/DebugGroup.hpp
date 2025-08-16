#pragma once

#include <SDL3/SDL.h>

#define CROBOTS_DEBUG_GROUP(commandBuffer) ::Crobots::DebugGroup(commandBuffer, __func__)

namespace Crobots
{

struct DebugGroup
{
    DebugGroup(SDL_GPUCommandBuffer* commandBuffer, const char* name);
    ~DebugGroup();

    SDL_GPUCommandBuffer* m_commandBuffer;
};

}