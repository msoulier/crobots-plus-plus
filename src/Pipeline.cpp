#include <SDL3/SDL.h>

#include <cstdint>
#include <cstddef>

#include "Log.hpp"
#include "ParticleBuffer.hpp"
#include "Pipeline.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "Window.hpp"

namespace Crobots
{

SDL_GPUGraphicsPipeline* CreateModelVoxObjPipeline(SDL_GPUDevice* device, Window& window)
{
    SDL_GPUShader* fragShader = LoadShader(device, "ModelVoxObj.frag");
    SDL_GPUShader* vertShader = LoadShader(device, "ModelVoxObj.vert");
    if (!fragShader || !vertShader)
    {
        CROBOTS_LOG("Failed to load shader(s)");
        return nullptr;
    }
    SDL_GPUColorTargetDescription targets[1]{};
    SDL_GPUVertexBufferDescription buffers[1]{};
    SDL_GPUVertexAttribute attribs[1]{};
    targets[0].format = SDL_GetGPUSwapchainTextureFormat(device, window.GetHandle());
    buffers[0].pitch = sizeof(uint64_t);
    attribs[0].format = SDL_GPU_VERTEXELEMENTFORMAT_UINT2;
    SDL_GPUGraphicsPipelineCreateInfo info{};
    info.vertex_shader = vertShader;
    info.fragment_shader = fragShader;
    info.target_info.color_target_descriptions = targets;
    info.target_info.num_color_targets = 1;
    info.target_info.depth_stencil_format = GetDepthTextureFormat(device);
    info.target_info.has_depth_stencil_target = true;
    info.vertex_input_state.vertex_buffer_descriptions = buffers;
    info.vertex_input_state.num_vertex_buffers = 1;
    info.vertex_input_state.vertex_attributes = attribs;
    info.vertex_input_state.num_vertex_attributes = 1;
    info.depth_stencil_state.enable_depth_test = true;
    info.depth_stencil_state.enable_depth_write = true;
    info.depth_stencil_state.compare_op = SDL_GPU_COMPAREOP_LESS;
    info.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_BACK;
    info.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
    SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(device, &info);
    if (!pipeline)
    {
        CROBOTS_LOG("Failed to create graphics pipeline(s): %s", SDL_GetError());
        return nullptr;
    }
    SDL_ReleaseGPUShader(device, fragShader);
    SDL_ReleaseGPUShader(device, vertShader);
    return pipeline;
}

SDL_GPUGraphicsPipeline* CreateParticlePipeline(SDL_GPUDevice* device, Window& window)
{
    SDL_GPUShader* fragShader = LoadShader(device, "Particle.frag");
    SDL_GPUShader* vertShader = LoadShader(device, "Particle.vert");
    if (!fragShader || !vertShader)
    {
        CROBOTS_LOG("Failed to load shader(s)");
        return nullptr;
    }
    SDL_GPUColorTargetDescription targets[1]{};
    SDL_GPUVertexBufferDescription buffers[2]{};
    SDL_GPUVertexAttribute attribs[5]{};
    targets[0].format = SDL_GetGPUSwapchainTextureFormat(device, window.GetHandle());
    buffers[0].slot = 0;
    buffers[0].pitch = sizeof(glm::vec3);
    buffers[0].input_rate = SDL_GPU_VERTEXINPUTRATE_INSTANCE;
    buffers[0].instance_step_rate = 0;
    buffers[1].slot = 1;
    buffers[1].pitch = sizeof(Particle);
    buffers[1].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    buffers[1].instance_step_rate = 0;
    attribs[0].location = 0;
    attribs[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    attribs[0].offset = 0;
    attribs[0].buffer_slot = 0;
    attribs[1].location = 1;
    attribs[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    attribs[1].offset = offsetof(Particle, position);
    attribs[1].buffer_slot = 1;
    attribs[2].location = 2;
    attribs[2].format = SDL_GPU_VERTEXELEMENTFORMAT_UINT;
    attribs[2].offset = offsetof(Particle, color);
    attribs[2].buffer_slot = 1;
    attribs[3].location = 3;
    attribs[3].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    attribs[3].offset = offsetof(Particle, velocity);
    attribs[3].buffer_slot = 1;
    attribs[4].location = 4;
    attribs[4].format = SDL_GPU_VERTEXELEMENTFORMAT_UINT;
    attribs[4].offset = offsetof(Particle, lifetime);
    attribs[4].buffer_slot = 1;
    SDL_GPUGraphicsPipelineCreateInfo info{};
    info.vertex_shader = vertShader;
    info.fragment_shader = fragShader;
    info.target_info.color_target_descriptions = targets;
    info.target_info.num_color_targets = 1;
    info.target_info.depth_stencil_format = GetDepthTextureFormat(device);
    info.target_info.has_depth_stencil_target = true;
    info.vertex_input_state.vertex_buffer_descriptions = buffers;
    info.vertex_input_state.num_vertex_buffers = 2;
    info.vertex_input_state.vertex_attributes = attribs;
    info.vertex_input_state.num_vertex_attributes = 5;
    info.depth_stencil_state.enable_depth_test = true;
    info.depth_stencil_state.enable_depth_write = true;
    info.depth_stencil_state.compare_op = SDL_GPU_COMPAREOP_LESS;
    info.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_BACK;
    info.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
    SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(device, &info);
    if (!pipeline)
    {
        CROBOTS_LOG("Failed to create graphics pipeline(s): %s", SDL_GetError());
        return nullptr;
    }
    SDL_ReleaseGPUShader(device, fragShader);
    SDL_ReleaseGPUShader(device, vertShader);
    return pipeline;
}

}