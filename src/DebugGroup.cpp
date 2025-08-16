#include <SDL3/SDL.h>
#include <SDLx_gpu/SDL_gpu.h>

#include "DebugGroup.hpp"

namespace Crobots
{

DebugGroup::DebugGroup(SDL_GPUCommandBuffer* commandBuffer, const char* name) : m_commandBuffer{commandBuffer}
{
    SDLx_GPUBeginDebugGroup(m_commandBuffer, name);
}

DebugGroup::~DebugGroup()
{
    SDLx_GPUEndDebugGroup(m_commandBuffer);
}

}