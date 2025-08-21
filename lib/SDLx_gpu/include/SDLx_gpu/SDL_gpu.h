#pragma once

#include <SDL3/SDL.h>
#include <SDLx_model/SDL_model.h>

/*
 * Misc
 */

SDL_GPUDevice* SDLx_GPUCreateDevice(bool low_power);
void SDLx_GPUBeginDebugGroup(SDL_GPUCommandBuffer* command_buffer, const char* name);
void SDLx_GPUEndDebugGroup(SDL_GPUCommandBuffer* command_buffer);
void SDLx_GPUInsertDebugLabel(SDL_GPUCommandBuffer* command_buffer, const char* name);
bool SDLx_GPUBeginFrame(SDL_GPUDevice* device, SDL_Window* window, SDL_GPUCommandBuffer** command_buffer,
    SDL_GPUTexture** swapchain_texture, uint32_t* width, uint32_t* height);
void SDLx_GPUClear(SDL_GPUCommandBuffer* command_buffer, SDL_GPUTexture* color_texture, SDL_GPUTexture* depth_texture);
SDL_GPUShader* SDLx_GPULoadShader(SDL_GPUDevice* device, const char* name);
SDL_GPUComputePipeline* SDLx_GPULoadComputePipeline(SDL_GPUDevice* device, const char* name);
SDL_GPUTexture* SDLx_GPULoadTexture(SDL_GPUDevice* device, SDL_GPUCopyPass* copy_pass, const char* path);
SDL_GPUTextureFormat SDLx_GPUGetDepthTextureFormat(SDL_GPUDevice* device);
SDL_GPUTextureFormat SDLx_GPUGetColorTextureFormat(SDL_GPUDevice* device);
SDL_GPUTexture* SDLx_GPUCreateColorTexture(SDL_GPUDevice* device, int width, int height, SDL_GPUTextureUsageFlags usage);
SDL_GPUTexture* SDLx_GPUCreateDepthTexture(SDL_GPUDevice* device, int width, int height, SDL_GPUTextureUsageFlags usage);
SDL_GPUSampler* SDLx_GPUCreateNearestSampler(SDL_GPUDevice* device);
SDL_GPUSampler* SDLx_GPUCreateLinearSampler(SDL_GPUDevice* device);

/*
 * Renderer
 */

typedef struct SDLx_GPURenderer SDLx_GPURenderer;

SDLx_GPURenderer* SDLx_GPUCreateRenderer(SDL_GPUDevice* device);
void SDLx_GPUDestroyRenderer(SDLx_GPURenderer* renderer);
void SDLx_GPURenderLine2D(SDLx_GPURenderer* renderer, float x1, float y1, float x2, float y2, Uint32 color);
void SDLx_GPURenderLine3D(SDLx_GPURenderer* renderer, float x1, float y1, float z1, float x2, float y2, float z2, Uint32 color);
void SDLx_GPURenderText2D(SDLx_GPURenderer* renderer, const char* path, const char* string, float x, float y, int size, Uint32 color);
void SDLx_GPURenderModel(SDLx_GPURenderer* renderer, const char* path, const void* transform, SDLx_ModelType type);
SDLx_Model* SDLx_GPUGetModel(SDLx_GPURenderer* renderer, const char* path, SDLx_ModelType type);
void SDLx_GPUSubmitRenderer(SDLx_GPURenderer* renderer, SDL_GPUCommandBuffer* command_buffer,
    SDL_GPUTexture* color_texture, SDL_GPUTexture* depth_texture, const void* matrix_2d, const void* matrix_3d);