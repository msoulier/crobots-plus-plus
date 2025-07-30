#include <SDL3/SDL.h>

#include "DebugGroup.hpp"

#ifndef NDEBUG
namespace Crobots
{

DebugGroup::DebugGroup(SDL_GPUCommandBuffer* commandBuffer, const char* name) : m_commandBuffer{commandBuffer}
{
    SDL_PushGPUDebugGroup(m_commandBuffer, name);
}

DebugGroup::~DebugGroup()
{
    SDL_PopGPUDebugGroup(m_commandBuffer);
}

}
#endif