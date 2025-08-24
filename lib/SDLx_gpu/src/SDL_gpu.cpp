#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDLx_gpu/SDL_gpu.h>
#include <SDLx_model/SDL_model.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <exception>
#include <format>
#include <fstream>
#include <iterator>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "json.hpp"
#include "stb_image.h"

#ifdef NDEBUG
#define DEBUG 0
#else
#define DEBUG 1
#endif

static constexpr size_t NullTextId = std::numeric_limits<size_t>::max();

struct DebugGroup
{
    DebugGroup(SDL_GPUCommandBuffer* command_buffer, const char* name)
        : command_buffer{command_buffer}
    {
        SDLx_GPUBeginDebugGroup(command_buffer, name);
    }

    ~DebugGroup()
    {
        SDLx_GPUEndDebugGroup(command_buffer);
    }

    SDL_GPUCommandBuffer* command_buffer;
};

struct Buffer
{
    Buffer()
        : transfer_buffer{nullptr}
        , buffer{nullptr}
        , transfer_buffer_capacity{0}
        , buffer_capacity{0}
        , transfer_buffer_size{0}
        , buffer_size{0} {}

    void Destroy(SDL_GPUDevice* device)
    {
        SDL_ReleaseGPUTransferBuffer(device, transfer_buffer);
        SDL_ReleaseGPUBuffer(device, buffer);
    }

    template<typename T>
    void Push(SDL_GPUDevice* device, const T& item)
    {
        /* NOTE: for ensuring vec3s are aligned to vec4s */
        static_assert(sizeof(T) % 16 == 0);
        if (!data && transfer_buffer)
        {
            buffer_size = 0;
            SDL_assert(!transfer_buffer_size);
            data = static_cast<uint8_t*>(SDL_MapGPUTransferBuffer(device, transfer_buffer, true));
            if (!data)
            {
                SDL_Log("Failed to map transfer buffer: %s", SDL_GetError());
                return;
            }
        }
        SDL_assert(transfer_buffer_size <= transfer_buffer_capacity);
        if (transfer_buffer_size == transfer_buffer_capacity)
        {
            uint32_t new_capacity = std::max(1024u, transfer_buffer_size * 2u);
            SDL_GPUTransferBufferCreateInfo info{};
            info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
            info.size = new_capacity;
            SDL_GPUTransferBuffer* new_transfer_buffer = SDL_CreateGPUTransferBuffer(device, &info);
            if (!new_transfer_buffer)
            {
                SDL_Log("Failed to create transfer buffer: %s", SDL_GetError());
                return;
            }
            uint8_t* new_data = static_cast<uint8_t*>(SDL_MapGPUTransferBuffer(device, new_transfer_buffer, false));
            if (!new_data)
            {
                SDL_Log("Failed to map transfer buffer: %s", SDL_GetError());
                SDL_ReleaseGPUTransferBuffer(device, new_transfer_buffer);
                return;
            }
            if (data)
            {
                std::copy(data, data + transfer_buffer_size, new_data);
                SDL_UnmapGPUTransferBuffer(device, transfer_buffer);
            }
            SDL_ReleaseGPUTransferBuffer(device, transfer_buffer);
            transfer_buffer_capacity = new_capacity;
            transfer_buffer = new_transfer_buffer;
            data = new_data;
        }
        SDL_assert(data);
        std::memcpy(data + transfer_buffer_size, std::addressof(item), sizeof(item));
        transfer_buffer_size += sizeof(item);
    }

    void Upload(SDL_GPUDevice* device, SDL_GPUCopyPass* copy_pass)
    {
        if (data)
        {
            SDL_UnmapGPUTransferBuffer(device, transfer_buffer);
            data = nullptr;
        }
        uint32_t size = transfer_buffer_size;
        transfer_buffer_size = 0;
        if (!size)
        {
            buffer_size = 0;
            return;
        }
        if (transfer_buffer_capacity > buffer_capacity)
        {
            SDL_ReleaseGPUBuffer(device, buffer);
            buffer = nullptr;
            buffer_capacity = 0;
            SDL_GPUBufferCreateInfo info{};
            info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
            info.size = transfer_buffer_capacity;
            buffer = SDL_CreateGPUBuffer(device, &info);
            if (!buffer)
            {
                SDL_Log("Failed to create buffer: %s", SDL_GetError());
                return;
            }
            buffer_capacity = transfer_buffer_capacity;
        }
        SDL_GPUTransferBufferLocation location{};
        SDL_GPUBufferRegion region{};
        location.transfer_buffer = transfer_buffer;
        region.buffer = buffer;
        region.size = size;
        SDL_UploadToGPUBuffer(copy_pass, &location, &region, true);
        buffer_size = size;
    }

    SDL_GPUBuffer* GetBuffer() const
    {
        return buffer;
    }

    uint32_t GetSize() const
    {
        return buffer_size;
    }

    SDL_GPUTransferBuffer* transfer_buffer;
    SDL_GPUBuffer* buffer;
    uint32_t transfer_buffer_capacity;
    uint32_t buffer_capacity;
    uint32_t transfer_buffer_size;
    uint32_t buffer_size;
    uint8_t* data;
};

enum class DrawType
{
    Line2D,
    Line3D,
};

struct DrawPass
{
    DrawType type;
    uint32_t offset;
};

struct Vertex2D
{
    float x;
    float y;
    uint32_t color;
    float padding;
};

struct Vertex3D
{
    float x;
    float y;
    float z;
    uint32_t color;
};

struct TextVertex
{
    float x;
    float y;
    float u;
    float v;
};

struct TextPass
{
    SDL_GPUTexture* atlas_texture;
    Uint32 vertex_offset;
    Uint16 first_index;
    Uint16 num_indices;
};

struct Text
{
    TTF_Text* handle;
    SDL_GPUBuffer* vertex_buffer;
    SDL_GPUBuffer* index_buffer;
    std::vector<TextPass> passes;
};

struct TextInstance2D
{
    size_t id;
    struct
    {
        float x;
        float y;
        uint32_t color;
    }
    data;
};

struct TextInstance3D
{
    size_t id;
    struct
    {
        float transform[16];
        uint32_t color;
    }
    data;
};

struct ModelData
{
    std::string path;
    SDLx_ModelType type;
};

struct ModelInstance
{
    ModelData data;
    float transform[16];
};

struct Font
{
    TTF_Font* handle;
    std::unordered_map<std::string, size_t> texts;
};

typedef struct SDLx_GPURenderer
{
    SDL_GPUDevice* device;
    SDL_GPUGraphicsPipeline* line_2d_pipeline;
    SDL_GPUGraphicsPipeline* line_3d_pipeline;
    SDL_GPUGraphicsPipeline* text_2d_pipeline;
    SDL_GPUGraphicsPipeline* text_3d_pipeline;
    SDL_GPUGraphicsPipeline* vox_obj_pipeline;
    SDL_GPUGraphicsPipeline* vox_raw_pipeline;
    SDL_GPUSampler* nearest_sampler;
    TTF_TextEngine* text_engine;
    Buffer buffer_2d;
    Buffer buffer_line_3d;
    std::vector<DrawPass> draw_passes_2d;
    std::unordered_map<std::string, std::unordered_map<int, Font>> fonts;
    std::unordered_map<std::string, std::array<SDLx_Model*, SDLX_MODELTYPE_COUNT>> models;
    std::vector<TextInstance2D> text_instances_2d;
    std::vector<TextInstance3D> text_instances_3d;
    std::vector<Text> texts;
    std::vector<ModelInstance> model_instances;
    std::vector<ModelData> model_requests;
} SDLx_GPURenderer;

SDL_GPUDevice* SDLx_GPUCreateDevice(bool low_power)
{
    /* TODO: waiting on https://github.com/libsdl-org/SDL/issues/12056 for debugging */
    SDL_PropertiesID props = SDL_GetGlobalProperties();
    SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_PREFERLOWPOWER_BOOLEAN, low_power);
    SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_DEBUGMODE_BOOLEAN, DEBUG);
    SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_VERBOSE_BOOLEAN, DEBUG);
#if SDL_PLATFORM_APPLE
    SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_SHADERS_MSL_BOOLEAN, true);
#elif defined(SDL_PLATFORM_WIN32) && DEBUG == 1
    SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_SHADERS_SPIRV_BOOLEAN, true);
#elif defined(SDL_PLATFORM_WIN32) && DEBUG == 0
    SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_SHADERS_DXIL_BOOLEAN, true);
#elif defined(SDL_PLATFORM_LINUX)
    SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_SHADERS_SPIRV_BOOLEAN, true);
#else
    static_assert(false, "Not supported")
#endif
#if defined(SDL_PLATFORM_WIN32) && DEBUG == 0
    SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_R_FLOAT, 0.0f);
    SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_G_FLOAT, 0.0f);
    SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_B_FLOAT, 0.0f);
    SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_A_FLOAT, 0.0f);
    SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_DEPTH_FLOAT, 1.0f);
    SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_STENCIL_NUMBER, 0.0f);
#endif
#if defined(SDL_PLATFORM_WIN32) && DEBUG == 0
    SDL_Log("Disabling debug groups and labels");
#endif
    SDL_GPUDevice* device = SDL_CreateGPUDeviceWithProperties(props);
    if (!device)
    {
        SDL_Log("Failed to create device: %s", SDL_GetError());
        return nullptr;
    }
    return device;
}

void SDLx_GPUBeginDebugGroup(SDL_GPUCommandBuffer* command_buffer, const char* name)
{
#if !defined(SDL_PLATFORM_WIN32) || DEBUG == 1
    SDL_PushGPUDebugGroup(command_buffer, name);
#endif
}

void SDLx_GPUEndDebugGroup(SDL_GPUCommandBuffer* command_buffer)
{
#if !defined(SDL_PLATFORM_WIN32) || DEBUG == 1
    SDL_PopGPUDebugGroup(command_buffer);
#endif
}

void SDLx_GPUInsertDebugLabel(SDL_GPUCommandBuffer* command_buffer, const char* name)
{
#if !defined(SDL_PLATFORM_WIN32) || DEBUG == 1
    SDL_InsertGPUDebugLabel(command_buffer, name);
#endif
}

bool SDLx_GPUBeginFrame(SDL_GPUDevice* device, SDL_Window* window, SDL_GPUCommandBuffer** command_buffer,
    SDL_GPUTexture** swapchain_texture, uint32_t* width, uint32_t* height)
{
    if (!device)
    {
        return SDL_InvalidParamError("device");
    }
    if (!window)
    {
        return SDL_InvalidParamError("window");
    }
    if (!command_buffer)
    {
        return SDL_InvalidParamError("command_buffer");
    }
    if (!swapchain_texture)
    {
        return SDL_InvalidParamError("swapchain_texture");
    }
    if (!width)
    {
        return SDL_InvalidParamError("width");
    }
    if (!height)
    {
        return SDL_InvalidParamError("height");
    }
    *command_buffer = SDL_AcquireGPUCommandBuffer(device);
    if (!*command_buffer)
    {
        SDL_Log("Failed to acquire command buffer: %s", SDL_GetError());
        return false;
    }
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(*command_buffer, window, swapchain_texture, width, height))
    {
        SDL_Log("Failed to acquire swapchain texture: %s", SDL_GetError());
        SDL_CancelGPUCommandBuffer(*command_buffer);
        return false;
    }
    if (!*swapchain_texture || !*width || !*height)
    {
        /* NOTE: not an error */
        SDL_SubmitGPUCommandBuffer(*command_buffer);
        return false;
    }
    return true;
}

void SDLx_GPUClear(SDL_GPUCommandBuffer* command_buffer, SDL_GPUTexture* color_texture, SDL_GPUTexture* depth_texture)
{
    if (!command_buffer)
    {
        SDL_InvalidParamError("command_buffer");
        return;
    }
    if (!color_texture)
    {
        SDL_InvalidParamError("color_texture");
        return;
    }
    SDL_GPUColorTargetInfo color_info{};
    color_info.texture = color_texture;
    color_info.load_op = SDL_GPU_LOADOP_CLEAR;
    color_info.store_op = SDL_GPU_STOREOP_STORE;
    SDL_GPURenderPass* render_pass;
    if (depth_texture)
    {
        SDL_GPUDepthStencilTargetInfo depth_info{};
        depth_info.texture = depth_texture;
        depth_info.clear_depth = 1.0f;
        depth_info.load_op = SDL_GPU_LOADOP_CLEAR;
        depth_info.stencil_load_op = SDL_GPU_LOADOP_CLEAR;
        depth_info.store_op = SDL_GPU_STOREOP_STORE;
        render_pass = SDL_BeginGPURenderPass(command_buffer, &color_info, 1, &depth_info);
        if (!render_pass)
        {
            SDL_Log("Failed to begin render pass: %s", SDL_GetError());
            return;
        }
    }
    else
    {
        render_pass = SDL_BeginGPURenderPass(command_buffer, &color_info, 1, nullptr);
        if (!render_pass)
        {
            SDL_Log("Failed to begin render pass: %s", SDL_GetError());
            return;
        }
    }
    SDL_EndGPURenderPass(render_pass);
}

static void* LoadShader(SDL_GPUDevice* device, const char* name)
{
    if (!device)
    {
        SDL_InvalidParamError("device");
        return nullptr;
    }
    if (!name)
    {
        SDL_InvalidParamError("name");
        return nullptr;
    }
    SDL_GPUShaderFormat shader_format = SDL_GetGPUShaderFormats(device);
    const char* entrypoint;
    const char* file_extension;
    if (shader_format & SDL_GPU_SHADERFORMAT_SPIRV)
    {
        shader_format = SDL_GPU_SHADERFORMAT_SPIRV;
        entrypoint = "main";
        file_extension = "spv";
    }
    else if (shader_format & SDL_GPU_SHADERFORMAT_DXIL)
    {
        shader_format = SDL_GPU_SHADERFORMAT_DXIL;
        entrypoint = "main";
        file_extension = "dxil";
    }
    else if (shader_format & SDL_GPU_SHADERFORMAT_MSL)
    {
        shader_format = SDL_GPU_SHADERFORMAT_MSL;
        entrypoint = "main0";
        file_extension = "msl";
    }
    else
    {
        SDL_assert(false);
    }
    std::string shader_path = std::format("{}.{}", name, file_extension);
    std::ifstream shader_file(shader_path, std::ios::binary);
    if (shader_file.fail())
    {
        SDL_Log("Failed to open shader: %s", shader_path.data());
        return nullptr;
    }
    std::string json_path = std::format("{}.json", name);
    std::ifstream json_file(json_path, std::ios::binary);
    if (json_file.fail())
    {
        SDL_Log("Failed to open json: %s", json_path.data());
        return nullptr;
    }
    std::string shader_data(std::istreambuf_iterator<char>(shader_file), {});
    nlohmann::json json;
    try
    {
        json_file >> json;
    }
    catch (const std::exception& exception)
    {
        SDL_Log("Failed to parse json: %s, %s", json_path.data(), exception.what());
        return nullptr;
    }
    void* shader = nullptr;
    if (std::strstr(name, ".comp"))
    {
        SDL_GPUComputePipelineCreateInfo info{};
        info.num_samplers = json["samplers"];
        info.num_readonly_storage_textures = json["readonly_storage_textures"];
        info.num_readonly_storage_buffers = json["readonly_storage_buffers"];
        info.num_readwrite_storage_textures = json["readwrite_storage_textures"];
        info.num_readwrite_storage_buffers = json["readwrite_storage_buffers"];
        info.num_uniform_buffers = json["uniform_buffers"];
        info.threadcount_x = json["threadcount_x"];
        info.threadcount_y = json["threadcount_y"];
        info.threadcount_z = json["threadcount_z"];
        info.code = reinterpret_cast<Uint8*>(shader_data.data());
        info.code_size = shader_data.size();
        info.entrypoint = entrypoint;
        info.format = shader_format;
        shader = SDL_CreateGPUComputePipeline(device, &info);
    }
    else
    {
        SDL_GPUShaderCreateInfo info{};
        info.num_samplers = json["samplers"];
        info.num_storage_textures = json["storage_textures"];
        info.num_storage_buffers = json["storage_buffers"];
        info.num_uniform_buffers = json["uniform_buffers"];
        info.code = reinterpret_cast<Uint8*>(shader_data.data());
        info.code_size = shader_data.size();
        info.entrypoint = entrypoint;
        info.format = shader_format;
        if (std::strstr(name, ".frag"))
        {
            info.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
        }
        else
        {
            info.stage = SDL_GPU_SHADERSTAGE_VERTEX;
        }
        shader = SDL_CreateGPUShader(device, &info);
    }
    if (!shader)
    {
        SDL_Log("Failed to create shader: %s, %s", name, SDL_GetError());
        return nullptr;
    }
    return shader;
}

SDL_GPUShader* SDLx_GPULoadShader(SDL_GPUDevice* device, const char* name)
{
    return static_cast<SDL_GPUShader*>(LoadShader(device, name));
}

SDL_GPUComputePipeline* SDLx_GPULoadComputePipeline(SDL_GPUDevice* device, const char* name)
{
    return static_cast<SDL_GPUComputePipeline*>(LoadShader(device, name));
}

SDL_GPUTexture* SDLx_GPULoadTexture(SDL_GPUDevice* device, SDL_GPUCopyPass* copy_pass, const char* path)
{
    if (!device)
    {
        SDL_InvalidParamError("device");
        return nullptr;
    }
    if (!copy_pass)
    {
        SDL_InvalidParamError("copy_pass");
        return nullptr;
    }
    if (!path)
    {
        SDL_InvalidParamError("path");
        return nullptr;
    }
    int width;
    int height;
    int channels;
    void* src_data = stbi_load(path, &width, &height, &channels, 4);
    if (!src_data)
    {
        SDL_Log("Failed to load image: %s, %s", path, stbi_failure_reason());
        return nullptr;
    }
    SDL_GPUTexture* texture;
    SDL_GPUTransferBuffer* transfer_buffer;
    {
        SDL_GPUTextureCreateInfo info{};
        info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
        info.type = SDL_GPU_TEXTURETYPE_2D;
        info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
        info.width = width;
        info.height = height;
        info.layer_count_or_depth = 1;
        info.num_levels = 1;
        texture = SDL_CreateGPUTexture(device, &info);
        if (!texture)
        {
            SDL_Log("Failed to create texture: %s, %s", path, SDL_GetError());
            stbi_image_free(src_data);
            return nullptr;
        }
    }
    {
        SDL_GPUTransferBufferCreateInfo info{};
        info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        info.size = width * height * 4;
        transfer_buffer = SDL_CreateGPUTransferBuffer(device, &info);
        if (!transfer_buffer)
        {
            SDL_Log("Failed to create transfer buffer: %s, %s", path, SDL_GetError());
            stbi_image_free(src_data);
            SDL_ReleaseGPUTexture(device, texture);
            return nullptr;
        }
    }
    void* dst_data = SDL_MapGPUTransferBuffer(device, transfer_buffer, false);
    if (!dst_data)
    {
        SDL_Log("Failed to map transfer buffer: %s, %s", path, SDL_GetError());
        stbi_image_free(src_data);
        SDL_ReleaseGPUTexture(device, texture);
        return nullptr;
    }
    std::memcpy(dst_data, src_data, width * height * 4);
    stbi_image_free(src_data);
    SDL_UnmapGPUTransferBuffer(device, transfer_buffer);
    SDL_GPUTextureTransferInfo info{};
    SDL_GPUTextureRegion region{};
    info.transfer_buffer = transfer_buffer;
    region.texture = texture;
    region.w = width;
    region.h = height;
    region.d = 1;
    SDL_UploadToGPUTexture(copy_pass, &info, &region, true);
    SDL_ReleaseGPUTransferBuffer(device, transfer_buffer);
    return texture;
}

SDL_GPUTextureFormat SDLx_GPUGetDepthTextureFormat(SDL_GPUDevice* device)
{
    if (!device)
    {
        SDL_InvalidParamError("device");
        return SDL_GPU_TEXTUREFORMAT_INVALID;
    }
    static constexpr SDL_GPUTextureType Type = SDL_GPU_TEXTURETYPE_2D;
    static constexpr SDL_GPUTextureUsageFlags Usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
    if (SDL_GPUTextureSupportsFormat(device, SDL_GPU_TEXTUREFORMAT_D24_UNORM, Type, Usage))
    {
        return SDL_GPU_TEXTUREFORMAT_D24_UNORM;
    }
    else
    {
        return SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
    }
}

SDL_GPUTextureFormat SDLx_GPUGetColorTextureFormat(SDL_GPUDevice* device)
{
    return SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
}

SDL_GPUTexture* SDLx_GPUCreateColorTexture(SDL_GPUDevice* device, int width, int height, SDL_GPUTextureUsageFlags usage)
{
    if (!device)
    {
        SDL_InvalidParamError("device");
        return nullptr;
    }
    if (!width)
    {
        SDL_InvalidParamError("width");
        return nullptr;
    }
    if (!height)
    {
        SDL_InvalidParamError("height");
        return nullptr;
    }
    SDL_GPUTextureCreateInfo info{};
    info.type = SDL_GPU_TEXTURETYPE_2D;
    info.usage = usage | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
    info.format = SDLx_GPUGetColorTextureFormat(device);
    info.width = width;
    info.height = height;
    info.layer_count_or_depth = 1;
    info.num_levels = 1;
    info.props = SDL_GetGlobalProperties();
    SDL_GPUTexture* texture = SDL_CreateGPUTexture(device, &info);
    if (!texture)
    {
        SDL_Log("Failed to create texture: %s", SDL_GetError());
        return nullptr;
    }
    return texture;
}

SDL_GPUTexture* SDLx_GPUCreateDepthTexture(SDL_GPUDevice* device, int width, int height, SDL_GPUTextureUsageFlags usage)
{
    if (!device)
    {
        SDL_InvalidParamError("device");
        return nullptr;
    }
    if (!width)
    {
        SDL_InvalidParamError("width");
        return nullptr;
    }
    if (!height)
    {
        SDL_InvalidParamError("height");
        return nullptr;
    }
    SDL_GPUTextureCreateInfo info{};
    info.type = SDL_GPU_TEXTURETYPE_2D;
    info.usage = usage | SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
    info.format = SDLx_GPUGetDepthTextureFormat(device);
    info.width = width;
    info.height = height;
    info.layer_count_or_depth = 1;
    info.num_levels = 1;
    info.props = SDL_GetGlobalProperties();
    SDL_GPUTexture* texture = SDL_CreateGPUTexture(device, &info);
    if (!texture)
    {
        SDL_Log("Failed to create texture: %s", SDL_GetError());
        return nullptr;
    }
    return texture;
}

SDL_GPUSampler* SDLx_GPUCreateNearestSampler(SDL_GPUDevice* device)
{
    if (!device)
    {
        SDL_InvalidParamError("device");
        return nullptr;
    }
    SDL_GPUSamplerCreateInfo info{};
    info.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    info.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    info.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    info.min_filter = SDL_GPU_FILTER_NEAREST;
    info.mag_filter = SDL_GPU_FILTER_NEAREST;
    info.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
    SDL_GPUSampler* sampler = SDL_CreateGPUSampler(device, &info);
    if (!sampler)
    {
        SDL_Log("Failed to create sampler: %s", SDL_GetError());
        return nullptr;
    }
    return sampler;
}

SDL_GPUSampler* SDLx_GPUCreateLinearSampler(SDL_GPUDevice* device)
{
    if (!device)
    {
        SDL_InvalidParamError("device");
        return nullptr;
    }
    SDL_GPUSamplerCreateInfo info{};
    info.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    info.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    info.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    info.min_filter = SDL_GPU_FILTER_LINEAR;
    info.mag_filter = SDL_GPU_FILTER_LINEAR;
    info.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
    SDL_GPUSampler* sampler = SDL_CreateGPUSampler(device, &info);
    if (!sampler)
    {
        SDL_Log("Failed to create sampler: %s", SDL_GetError());
        return nullptr;
    }
    return sampler;
}

static SDL_GPUGraphicsPipeline* CreateText2DPipeline(SDL_GPUDevice* device)
{
    SDL_GPUShader* frag_shader = SDLx_GPULoadShader(device, "text_2d.frag");
    SDL_GPUShader* vert_shader = SDLx_GPULoadShader(device, "text_2d.vert");
    if (!frag_shader || !vert_shader)
    {
        SDL_Log("Failed to load shader(s)");
        return nullptr;
    }
    SDL_GPUColorTargetDescription targets[1]{};
    SDL_GPUVertexBufferDescription buffers[1]{};
    SDL_GPUVertexAttribute attribs[2]{};
    targets[0].format = SDLx_GPUGetColorTextureFormat(device);
    buffers[0].slot = 0;
    buffers[0].pitch = sizeof(TextVertex);
    buffers[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    buffers[0].instance_step_rate = 0;
    attribs[0].location = 0;
    attribs[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
    attribs[0].offset = offsetof(TextVertex, x);
    attribs[0].buffer_slot = 0;
    attribs[1].location = 1;
    attribs[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
    attribs[1].offset = offsetof(TextVertex, u);
    attribs[1].buffer_slot = 0;
    SDL_GPUGraphicsPipelineCreateInfo info{};
    info.vertex_shader = vert_shader;
    info.fragment_shader = frag_shader;
    info.target_info.color_target_descriptions = targets;
    info.target_info.num_color_targets = 1;
    info.target_info.depth_stencil_format = SDLx_GPUGetDepthTextureFormat(device);
    info.target_info.has_depth_stencil_target = true;
    info.vertex_input_state.vertex_buffer_descriptions = buffers;
    info.vertex_input_state.num_vertex_buffers = 1;
    info.vertex_input_state.vertex_attributes = attribs;
    info.vertex_input_state.num_vertex_attributes = 2;
    info.depth_stencil_state.enable_depth_test = false;
    SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(device, &info);
    if (!pipeline)
    {
        SDL_Log("Failed to create graphics pipeline: %s", SDL_GetError());
        return nullptr;
    }
    SDL_ReleaseGPUShader(device, frag_shader);
    SDL_ReleaseGPUShader(device, vert_shader);
    return pipeline;
}

static SDL_GPUGraphicsPipeline* CreateText3DPipeline(SDL_GPUDevice* device)
{
    SDL_GPUShader* frag_shader = SDLx_GPULoadShader(device, "text_3d.frag");
    SDL_GPUShader* vert_shader = SDLx_GPULoadShader(device, "text_3d.vert");
    if (!frag_shader || !vert_shader)
    {
        SDL_Log("Failed to load shader(s)");
        return nullptr;
    }
    SDL_GPUColorTargetDescription targets[1]{};
    SDL_GPUVertexBufferDescription buffers[1]{};
    SDL_GPUVertexAttribute attribs[2]{};
    targets[0].format = SDLx_GPUGetColorTextureFormat(device);
    buffers[0].slot = 0;
    buffers[0].pitch = sizeof(TextVertex);
    buffers[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    buffers[0].instance_step_rate = 0;
    attribs[0].location = 0;
    attribs[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
    attribs[0].offset = offsetof(TextVertex, x);
    attribs[0].buffer_slot = 0;
    attribs[1].location = 1;
    attribs[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
    attribs[1].offset = offsetof(TextVertex, u);
    attribs[1].buffer_slot = 0;
    SDL_GPUGraphicsPipelineCreateInfo info{};
    info.vertex_shader = vert_shader;
    info.fragment_shader = frag_shader;
    info.target_info.color_target_descriptions = targets;
    info.target_info.num_color_targets = 1;
    info.target_info.depth_stencil_format = SDLx_GPUGetDepthTextureFormat(device);
    info.target_info.has_depth_stencil_target = true;
    info.vertex_input_state.vertex_buffer_descriptions = buffers;
    info.vertex_input_state.num_vertex_buffers = 1;
    info.vertex_input_state.vertex_attributes = attribs;
    info.vertex_input_state.num_vertex_attributes = 2;
    info.depth_stencil_state.compare_op = SDL_GPU_COMPAREOP_LESS;
    info.depth_stencil_state.enable_depth_write = true;
    info.depth_stencil_state.enable_depth_test = true;
    SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(device, &info);
    if (!pipeline)
    {
        SDL_Log("Failed to create graphics pipeline: %s", SDL_GetError());
        return nullptr;
    }
    SDL_ReleaseGPUShader(device, frag_shader);
    SDL_ReleaseGPUShader(device, vert_shader);
    return pipeline;
}

static SDL_GPUGraphicsPipeline* CreateLine2DPipeline(SDL_GPUDevice* device)
{
    SDL_GPUShader* frag_shader = SDLx_GPULoadShader(device, "line_2d.frag");
    SDL_GPUShader* vert_shader = SDLx_GPULoadShader(device, "line_2d.vert");
    if (!frag_shader || !vert_shader)
    {
        SDL_Log("Failed to load shader(s)");
        return nullptr;
    }
    SDL_GPUColorTargetDescription targets[1]{};
    SDL_GPUVertexBufferDescription buffers[1]{};
    SDL_GPUVertexAttribute attribs[2]{};
    targets[0].format = SDLx_GPUGetColorTextureFormat(device);
    buffers[0].slot = 0;
    buffers[0].pitch = sizeof(Vertex2D);
    buffers[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    buffers[0].instance_step_rate = 0;
    attribs[0].location = 0;
    attribs[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
    attribs[0].offset = offsetof(Vertex2D, x);
    attribs[0].buffer_slot = 0;
    attribs[1].location = 1;
    attribs[1].format = SDL_GPU_VERTEXELEMENTFORMAT_UINT;
    attribs[1].offset = offsetof(Vertex2D, color);
    attribs[1].buffer_slot = 0;
    SDL_GPUGraphicsPipelineCreateInfo info{};
    info.vertex_shader = vert_shader;
    info.fragment_shader = frag_shader;
    info.target_info.color_target_descriptions = targets;
    info.target_info.num_color_targets = 1;
    info.target_info.depth_stencil_format = SDLx_GPUGetDepthTextureFormat(device);
    info.target_info.has_depth_stencil_target = true;
    info.vertex_input_state.vertex_buffer_descriptions = buffers;
    info.vertex_input_state.num_vertex_buffers = 1;
    info.vertex_input_state.vertex_attributes = attribs;
    info.vertex_input_state.num_vertex_attributes = 2;
    info.depth_stencil_state.enable_depth_test = false;
    info.primitive_type = SDL_GPU_PRIMITIVETYPE_LINELIST;
    SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(device, &info);
    if (!pipeline)
    {
        SDL_Log("Failed to create graphics pipeline: %s", SDL_GetError());
        return nullptr;
    }
    SDL_ReleaseGPUShader(device, frag_shader);
    SDL_ReleaseGPUShader(device, vert_shader);
    return pipeline;
}

static SDL_GPUGraphicsPipeline* CreateLine3DPipeline(SDL_GPUDevice* device)
{
    SDL_GPUShader* frag_shader = SDLx_GPULoadShader(device, "line_3d.frag");
    SDL_GPUShader* vert_shader = SDLx_GPULoadShader(device, "line_3d.vert");
    if (!frag_shader || !vert_shader)
    {
        SDL_Log("Failed to load shader(s)");
        return nullptr;
    }
    SDL_GPUColorTargetDescription targets[1]{};
    SDL_GPUVertexBufferDescription buffers[1]{};
    SDL_GPUVertexAttribute attribs[2]{};
    targets[0].format = SDLx_GPUGetColorTextureFormat(device);
    buffers[0].slot = 0;
    buffers[0].pitch = sizeof(Vertex3D);
    buffers[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    buffers[0].instance_step_rate = 0;
    attribs[0].location = 0;
    attribs[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    attribs[0].offset = offsetof(Vertex3D, x);
    attribs[0].buffer_slot = 0;
    attribs[1].location = 1;
    attribs[1].format = SDL_GPU_VERTEXELEMENTFORMAT_UINT;
    attribs[1].offset = offsetof(Vertex3D, color);
    attribs[1].buffer_slot = 0;
    SDL_GPUGraphicsPipelineCreateInfo info{};
    info.vertex_shader = vert_shader;
    info.fragment_shader = frag_shader;
    info.target_info.color_target_descriptions = targets;
    info.target_info.num_color_targets = 1;
    info.target_info.depth_stencil_format = SDLx_GPUGetDepthTextureFormat(device);
    info.target_info.has_depth_stencil_target = true;
    info.vertex_input_state.vertex_buffer_descriptions = buffers;
    info.vertex_input_state.num_vertex_buffers = 1;
    info.vertex_input_state.vertex_attributes = attribs;
    info.vertex_input_state.num_vertex_attributes = 2;
    info.depth_stencil_state.compare_op = SDL_GPU_COMPAREOP_LESS;
    info.depth_stencil_state.enable_depth_write = true;
    info.depth_stencil_state.enable_depth_test = true;
    info.primitive_type = SDL_GPU_PRIMITIVETYPE_LINELIST;
    SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(device, &info);
    if (!pipeline)
    {
        SDL_Log("Failed to create graphics pipeline: %s", SDL_GetError());
        return nullptr;
    }
    SDL_ReleaseGPUShader(device, frag_shader);
    SDL_ReleaseGPUShader(device, vert_shader);
    return pipeline;
}

SDL_GPUGraphicsPipeline* CreateVoxObjPipeline(SDL_GPUDevice* device)
{
    SDL_GPUShader* frag_shader = SDLx_GPULoadShader(device, "vox_obj.frag");
    SDL_GPUShader* vert_shader = SDLx_GPULoadShader(device, "vox_obj.vert");
    if (!frag_shader || !vert_shader)
    {
        SDL_Log("Failed to load shader(s)");
        return nullptr;
    }
    SDL_GPUColorTargetDescription targets[1]{};
    SDL_GPUVertexBufferDescription buffers[1]{};
    SDL_GPUVertexAttribute attribs[1]{};
    targets[0].format = SDLx_GPUGetColorTextureFormat(device);
    buffers[0].pitch = sizeof(SDLx_ModelVoxObjVertex);
    attribs[0].format = SDL_GPU_VERTEXELEMENTFORMAT_UINT2;
    SDL_GPUGraphicsPipelineCreateInfo info{};
    info.vertex_shader = vert_shader;
    info.fragment_shader = frag_shader;
    info.target_info.color_target_descriptions = targets;
    info.target_info.num_color_targets = 1;
    info.target_info.depth_stencil_format = SDLx_GPUGetDepthTextureFormat(device);
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
        SDL_Log("Failed to create graphics pipeline: %s", SDL_GetError());
        return nullptr;
    }
    SDL_ReleaseGPUShader(device, frag_shader);
    SDL_ReleaseGPUShader(device, vert_shader);
    return pipeline;
}

SDL_GPUGraphicsPipeline* CreateVoxRawPipeline(SDL_GPUDevice* device)
{
    SDL_GPUShader* frag_shader = SDLx_GPULoadShader(device, "vox_raw.frag");
    SDL_GPUShader* vert_shader = SDLx_GPULoadShader(device, "vox_raw.vert");
    if (!frag_shader || !vert_shader)
    {
        SDL_Log("Failed to load shader(s)");
        return nullptr;
    }
    SDL_GPUColorTargetDescription targets[1]{};
    SDL_GPUVertexBufferDescription buffers[2]{};
    SDL_GPUVertexAttribute attribs[3]{};
    targets[0].format = SDLx_GPUGetColorTextureFormat(device);
    buffers[0].slot = 0;
    buffers[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    buffers[0].instance_step_rate = 0;
    buffers[0].pitch = sizeof(SDLx_ModelVec3);
    buffers[1].slot = 1;
    buffers[1].input_rate = SDL_GPU_VERTEXINPUTRATE_INSTANCE;
    buffers[1].instance_step_rate = 0;
    buffers[1].pitch = sizeof(SDLx_ModelVoxRawInstance);
    attribs[0].location = 0;
    attribs[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    attribs[0].buffer_slot = 0;
    attribs[0].offset = 0;
    attribs[1].location = 1;
    attribs[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    attribs[1].buffer_slot = 1;
    attribs[1].offset = 0;
    attribs[2].location = 2;
    attribs[2].format = SDL_GPU_VERTEXELEMENTFORMAT_UINT;
    attribs[2].buffer_slot = 1;
    attribs[2].offset = offsetof(SDLx_ModelVoxRawInstance, color);
    SDL_GPUGraphicsPipelineCreateInfo info{};
    info.vertex_shader = vert_shader;
    info.fragment_shader = frag_shader;
    info.target_info.color_target_descriptions = targets;
    info.target_info.num_color_targets = 1;
    info.target_info.depth_stencil_format = SDLx_GPUGetDepthTextureFormat(device);
    info.target_info.has_depth_stencil_target = true;
    info.vertex_input_state.vertex_buffer_descriptions = buffers;
    info.vertex_input_state.num_vertex_buffers = 2;
    info.vertex_input_state.vertex_attributes = attribs;
    info.vertex_input_state.num_vertex_attributes = 3;
    info.depth_stencil_state.enable_depth_test = true;
    info.depth_stencil_state.enable_depth_write = true;
    info.depth_stencil_state.compare_op = SDL_GPU_COMPAREOP_LESS;
    info.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_BACK;
    info.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
    SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(device, &info);
    if (!pipeline)
    {
        SDL_Log("Failed to create graphics pipeline: %s", SDL_GetError());
        return nullptr;
    }
    SDL_ReleaseGPUShader(device, frag_shader);
    SDL_ReleaseGPUShader(device, vert_shader);
    return pipeline;
}

SDLx_GPURenderer* SDLx_GPUCreateRenderer(SDL_GPUDevice* device)
{
    if (!device)
    {
        SDL_InvalidParamError("device");
        return nullptr;
    }
    if (!TTF_Init())
    {
        SDL_Log("Failed to initialize SDL ttf: %s", SDL_GetError());
        return nullptr;
    }
    SDLx_GPURenderer* renderer = new SDLx_GPURenderer();
    if (!renderer)
    {
        SDL_Log("Failed to create renderer");
        return nullptr;
    }
    renderer->device = device;
    renderer->text_engine = TTF_CreateGPUTextEngine(device);
    if (!renderer->text_engine)
    {
        SDL_Log("Failed to create text engine: %s", SDL_GetError());
        return nullptr;
    }
    renderer->line_2d_pipeline = CreateLine2DPipeline(device);
    if (!renderer->line_2d_pipeline)
    {
        SDL_Log("Failed to create line 2d pipeline");
        return nullptr;
    }
    renderer->line_3d_pipeline = CreateLine3DPipeline(device);
    if (!renderer->line_3d_pipeline)
    {
        SDL_Log("Failed to create line 3d pipeline");
        return nullptr;
    }
    renderer->text_2d_pipeline = CreateText2DPipeline(device);
    if (!renderer->text_2d_pipeline)
    {
        SDL_Log("Failed to create text 2d pipeline");
        return nullptr;
    }
    renderer->text_3d_pipeline = CreateText3DPipeline(device);
    if (!renderer->text_3d_pipeline)
    {
        SDL_Log("Failed to create text 3d pipeline");
        return nullptr;
    }
    renderer->vox_obj_pipeline = CreateVoxObjPipeline(device);
    if (!renderer->vox_obj_pipeline)
    {
        SDL_Log("Failed to create vox obj pipeline");
        return nullptr;
    }
    renderer->vox_raw_pipeline = CreateVoxRawPipeline(device);
    if (!renderer->vox_raw_pipeline)
    {
        SDL_Log("Failed to create vox raw pipeline");
        return nullptr;
    }
    renderer->nearest_sampler = SDLx_GPUCreateNearestSampler(device);
    if (!renderer->nearest_sampler)
    {
        SDL_Log("Failed to create nearest sampler");
        return nullptr;
    }
    return renderer;
}

void SDLx_GPUDestroyRenderer(SDLx_GPURenderer* renderer)
{
    if (!renderer)
    {
        return;
    }
    for (Text& text : renderer->texts)
    {
        TTF_DestroyText(text.handle);
        SDL_ReleaseGPUBuffer(renderer->device, text.vertex_buffer);
        SDL_ReleaseGPUBuffer(renderer->device, text.index_buffer);
    }
    for (auto& [path, fonts] : renderer->fonts)
    for (auto& [size, font] : fonts)
    {
        TTF_CloseFont(font.handle);
    }
    for (auto& [path, models] : renderer->models)
    for (auto& model : models)
    {
        SDLx_ModelDestroy(renderer->device, model);
    }
    renderer->buffer_2d.Destroy(renderer->device);
    renderer->buffer_line_3d.Destroy(renderer->device);
    SDL_ReleaseGPUSampler(renderer->device, renderer->nearest_sampler);
    SDL_ReleaseGPUGraphicsPipeline(renderer->device, renderer->text_2d_pipeline);
    SDL_ReleaseGPUGraphicsPipeline(renderer->device, renderer->text_3d_pipeline);
    SDL_ReleaseGPUGraphicsPipeline(renderer->device, renderer->line_3d_pipeline);
    SDL_ReleaseGPUGraphicsPipeline(renderer->device, renderer->line_2d_pipeline);
    SDL_ReleaseGPUGraphicsPipeline(renderer->device, renderer->vox_obj_pipeline);
    SDL_ReleaseGPUGraphicsPipeline(renderer->device, renderer->vox_raw_pipeline);
    TTF_DestroyGPUTextEngine(renderer->text_engine);
    TTF_Quit();
    delete renderer;
}

template<typename T>
static void PushDraw2D(SDLx_GPURenderer* renderer, const T& data, DrawType type)
{
    std::vector<DrawPass>& passes = renderer->draw_passes_2d;
    if (passes.empty())
    {
        passes.emplace_back(type, 0);
    }
    else if (passes.back().type != type)
    {
        passes.emplace_back(type, renderer->buffer_2d.GetSize());
    }
    renderer->buffer_2d.Push(renderer->device, data);
}

void SDLx_GPURenderLine2D(SDLx_GPURenderer* renderer, float x1, float y1, float x2, float y2, Uint32 color)
{
    if (!renderer)
    {
        SDL_InvalidParamError("renderer");
        return;
    }
    Vertex2D vertex1{x1, y1, color};
    Vertex2D vertex2{x2, y2, color};
    PushDraw2D(renderer, vertex1, DrawType::Line2D);
    PushDraw2D(renderer, vertex2, DrawType::Line2D);
}

void SDLx_GPURenderLine3D(SDLx_GPURenderer* renderer, float x1, float y1, float z1, float x2, float y2, float z2, Uint32 color)
{
    if (!renderer)
    {
        SDL_InvalidParamError("renderer");
        return;
    }
    Vertex3D vertex1{x1, y1, z1, color};
    Vertex3D vertex2{x2, y2, z2, color};
    renderer->buffer_line_3d.Push(renderer->device, vertex1);
    renderer->buffer_line_3d.Push(renderer->device, vertex2);
}

static size_t PrepareText(SDLx_GPURenderer* renderer, const char* path, const char* string, int size)
{
    if (!renderer)
    {
        SDL_InvalidParamError("renderer");
        return NullTextId;
    }
    if (!path)
    {
        SDL_InvalidParamError("path");
        return NullTextId;
    }
    if (!string)
    {
        SDL_InvalidParamError("string");
        return NullTextId;
    }
    if (!size)
    {
        SDL_InvalidParamError("size");
        return NullTextId;
    }
    auto& fonts = renderer->fonts[path];
    auto font_it = fonts.find(size);
    if (font_it == fonts.end())
    {
        Font font;
        font.handle = TTF_OpenFont(path, size);
        if (!font.handle)
        {
            SDL_Log("Failed to open font: %s", SDL_GetError());
            return NullTextId;
        }
        font_it = fonts.emplace(size, font).first;
    }
    auto& texts = font_it->second.texts;
    auto text_it = texts.find(string);
    if (text_it == texts.end())
    {
        Text& text = renderer->texts.emplace_back();
        text.handle = TTF_CreateText(renderer->text_engine, font_it->second.handle, string, 0);
        if (!text.handle)
        {
            SDL_Log("Failed to create text: %s", SDL_GetError());
            return NullTextId;
        }
        text_it = texts.emplace(string, renderer->texts.size() - 1).first;
    }
    if (text_it == texts.end())
    {
        return NullTextId;
    }
    else
    {
        return text_it->second;
    }
}

void SDLx_GPURenderText2D(SDLx_GPURenderer* renderer, const char* path, const char* string, float x, float y, int size, Uint32 color)
{
    TextInstance2D instance;
    instance.id = PrepareText(renderer, path, string, size);
    instance.data.x = x;
    instance.data.y = y;
    instance.data.color = color;
    renderer->text_instances_2d.push_back(instance);
}

void SDLx_GPURenderText3D(SDLx_GPURenderer* renderer, const char* path, const char* string, const void* transform, int size, Uint32 color)
{
    TextInstance3D instance;
    instance.id = PrepareText(renderer, path, string, size);
    std::memcpy(instance.data.transform, transform, 64);
    instance.data.color = color;
    renderer->text_instances_3d.push_back(instance);
}

void SDLx_GPURenderModel(SDLx_GPURenderer* renderer, const char* path, const void* transform, SDLx_ModelType type)
{
    if (!renderer)
    {
        SDL_InvalidParamError("renderer");
        return;
    }
    if (!path)
    {
        SDL_InvalidParamError("path");
        return;
    }
    if (!transform)
    {
        SDL_InvalidParamError("transform");
        return;
    }
    if (type == SDLX_MODELTYPE_INVALID)
    {
        SDL_InvalidParamError("type");
        return;
    }
    ModelInstance instance;
    instance.data.path = path;
    instance.data.type = type;
    std::memcpy(instance.transform, transform, 64);
    renderer->model_instances.push_back(instance);
}

SDLx_Model* SDLx_GPUGetModel(SDLx_GPURenderer* renderer, const char* path, SDLx_ModelType type)
{
    if (!renderer)
    {
        SDL_InvalidParamError("renderer");
        return nullptr;
    }
    if (!path)
    {
        SDL_InvalidParamError("path");
        return nullptr;
    }
    if (type == SDLX_MODELTYPE_INVALID)
    {
        SDL_InvalidParamError("type");
        return nullptr;
    }
    SDLx_Model* model = renderer->models[path][type];
    if (!model)
    {
        renderer->model_requests.emplace_back(path, type);
    }
    return model;
}

static void UploadText(SDLx_GPURenderer* renderer, size_t id, SDL_GPUCopyPass* copy_pass)
{
    if (id == NullTextId)
    {
        return;
    }
    Text& text = renderer->texts[id];
    if (!text.passes.empty())
    {
        return;
    }
    TTF_GPUAtlasDrawSequence* head = TTF_GetGPUTextDrawData(text.handle);
    if (!head)
    {
        SDL_Log("Failed to get text draw data: %s", SDL_GetError());
        return;
    }
    uint32_t num_vertices = 0;
    uint32_t num_indices = 0;
    for (TTF_GPUAtlasDrawSequence* curr = head; curr; curr = curr->next)
    {
        num_vertices += curr->num_vertices;
        num_indices += curr->num_indices;
    }
    SDL_assert(num_indices < std::numeric_limits<uint16_t>::max());
    SDL_GPUTransferBuffer* vertex_transfer_buffer;
    SDL_GPUTransferBuffer* index_transfer_buffer;
    {
        SDL_GPUTransferBufferCreateInfo info{};
        info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        info.size = num_vertices * sizeof(TextVertex);
        vertex_transfer_buffer = SDL_CreateGPUTransferBuffer(renderer->device, &info);
        info.size = num_indices * sizeof(uint16_t);
        index_transfer_buffer = SDL_CreateGPUTransferBuffer(renderer->device, &info);
        if (!vertex_transfer_buffer || !index_transfer_buffer)
        {
            SDL_Log("Failed to create transfer buffer(s): %s", SDL_GetError());
            return;
        }
    }
    TextVertex* vertex_data = static_cast<TextVertex*>(SDL_MapGPUTransferBuffer(renderer->device, vertex_transfer_buffer, false));
    uint16_t* index_data = static_cast<uint16_t*>(SDL_MapGPUTransferBuffer(renderer->device, index_transfer_buffer, false));
    if (!vertex_data || !index_data)
    {
        SDL_Log("Failed to map transfer buffer(s): %s", SDL_GetError());
        return;
    }
    num_vertices = 0;
    num_indices = 0;
    for (TTF_GPUAtlasDrawSequence* curr = head; curr; curr = curr->next)
    {
        TextPass& pass = text.passes.emplace_back();
        pass.atlas_texture = curr->atlas_texture;
        pass.num_indices = curr->num_indices;
        pass.vertex_offset = num_vertices * sizeof(TextVertex);
        pass.first_index = num_indices;
        for (uint32_t i = 0; i < curr->num_vertices; i++)
        {
            vertex_data[num_vertices + i].x = curr->xy[i].x;
            vertex_data[num_vertices + i].y = curr->xy[i].y;
            vertex_data[num_vertices + i].u = curr->uv[i].x;
            vertex_data[num_vertices + i].v = curr->uv[i].y;
        }
        for (uint32_t i = 0; i < curr->num_indices; i++)
        {
            index_data[num_indices + i] = num_vertices + curr->indices[i];
        }
        num_vertices += curr->num_vertices;
        num_indices += curr->num_indices;
    }
    SDL_UnmapGPUTransferBuffer(renderer->device, vertex_transfer_buffer);
    SDL_UnmapGPUTransferBuffer(renderer->device, index_transfer_buffer);
    {
        SDL_GPUBufferCreateInfo info{};
        info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
        info.size = num_vertices * sizeof(TextVertex);
        text.vertex_buffer = SDL_CreateGPUBuffer(renderer->device, &info);
        info.usage = SDL_GPU_BUFFERUSAGE_INDEX;
        info.size = num_indices * sizeof(uint16_t);
        text.index_buffer = SDL_CreateGPUBuffer(renderer->device, &info);
        if (!text.vertex_buffer || !text.index_buffer)
        {
            SDL_Log("Failed to create buffer(s): %s", SDL_GetError());
            return;
        }
    }
    {
        SDL_GPUTransferBufferLocation location{};
        SDL_GPUBufferRegion region{};
        location.transfer_buffer = vertex_transfer_buffer;
        region.buffer = text.vertex_buffer;
        region.size = num_vertices * sizeof(TextVertex);
        SDL_UploadToGPUBuffer(copy_pass, &location, &region, false);
        location.transfer_buffer = index_transfer_buffer;
        region.buffer = text.index_buffer;
        region.size = num_indices * sizeof(uint16_t);
        SDL_UploadToGPUBuffer(copy_pass, &location, &region, false);
    }
    SDL_ReleaseGPUTransferBuffer(renderer->device, vertex_transfer_buffer);
    SDL_ReleaseGPUTransferBuffer(renderer->device, index_transfer_buffer);
}

static void RenderShapes2D(SDLx_GPURenderer* renderer, SDL_GPUCommandBuffer* command_buffer,
    SDL_GPURenderPass* render_pass, const void* matrix_2d, const void* matrix_3d)
{
    DebugGroup debug_group(command_buffer, "SDLx_gpu::RenderShapes2D");
    std::vector<DrawPass>& passes = renderer->draw_passes_2d;
    for (int i = 0; i < passes.size(); i++)
    {
        DrawPass& pass = passes[i];
        switch (pass.type)
        {
        case DrawType::Line2D:
            SDL_BindGPUGraphicsPipeline(render_pass, renderer->line_2d_pipeline);
            break;
        default:
            SDL_assert(false);
        }
        uint32_t size;
        if (i == passes.size() - 1)
        {
            size = renderer->buffer_2d.GetSize() - pass.offset;
        }
        else
        {
            size = passes[i + 1].offset - pass.offset;
        }
        SDL_GPUBufferBinding vertex_buffer{};
        vertex_buffer.buffer = renderer->buffer_2d.GetBuffer();
        vertex_buffer.offset = pass.offset;
        SDL_PushGPUVertexUniformData(command_buffer, 0, matrix_2d, 64);
        SDL_BindGPUVertexBuffers(render_pass, 0, &vertex_buffer, 1);
        SDL_DrawGPUPrimitives(render_pass, size / sizeof(Vertex2D), 1, 0, 0);
    }
}

static void RenderShapes3D(SDLx_GPURenderer* renderer, SDL_GPUCommandBuffer* command_buffer,
    SDL_GPURenderPass* render_pass, const void* matrix_2d, const void* matrix_3d)
{
    DebugGroup debug_group(command_buffer, "SDLx_gpu::RenderShapes3D");
    if (renderer->buffer_line_3d.GetSize())
    {
        SDL_GPUBufferBinding vertex_buffer{};
        vertex_buffer.buffer = renderer->buffer_line_3d.GetBuffer();
        SDL_BindGPUGraphicsPipeline(render_pass, renderer->line_3d_pipeline);
        SDL_PushGPUVertexUniformData(command_buffer, 0, matrix_3d, 64);
        SDL_BindGPUVertexBuffers(render_pass, 0, &vertex_buffer, 1);
        SDL_DrawGPUPrimitives(render_pass, renderer->buffer_line_3d.GetSize() / sizeof(Vertex3D), 1, 0, 0);
    }
}

static void RenderText2D(SDLx_GPURenderer* renderer, SDL_GPUCommandBuffer* command_buffer,
    SDL_GPURenderPass* render_pass, const void* matrix_2d, const void* matrix_3d)
{
    DebugGroup debug_group(command_buffer, "SDLx_gpu::RenderText2D");
    SDL_BindGPUGraphicsPipeline(render_pass, renderer->text_2d_pipeline);
    SDL_PushGPUVertexUniformData(command_buffer, 0, matrix_2d, 64);
    for (TextInstance2D& instance : renderer->text_instances_2d)
    {
        if (instance.id == NullTextId)
        {
            continue;
        }
        Text& text = renderer->texts[instance.id];
        if (text.passes.empty())
        {
            continue;
        }
        int width;
        int height;
        TTF_GetTextSize(text.handle, &width, &height);
        instance.data.x -= width / 2;
        instance.data.y += height / 2;
        SDL_PushGPUVertexUniformData(command_buffer, 1, &instance.data, sizeof(instance.data));
        SDL_GPUBufferBinding vertex_buffer{};
        SDL_GPUBufferBinding index_buffer{};
        SDL_GPUTextureSamplerBinding atlas_texture{};
        vertex_buffer.buffer = text.vertex_buffer;
        index_buffer.buffer = text.index_buffer;
        atlas_texture.sampler = renderer->nearest_sampler;
        SDL_BindGPUIndexBuffer(render_pass, &index_buffer, SDL_GPU_INDEXELEMENTSIZE_16BIT);
        for (const TextPass& pass : text.passes)
        {
            vertex_buffer.offset = pass.vertex_offset;
            atlas_texture.texture = pass.atlas_texture;
            SDL_BindGPUVertexBuffers(render_pass, 0, &vertex_buffer, 1);
            SDL_BindGPUFragmentSamplers(render_pass, 0, &atlas_texture, 1);
            SDL_DrawGPUIndexedPrimitives(render_pass, pass.num_indices, 1, pass.first_index, 0, 0);
        }
    }
}

static void RenderText3D(SDLx_GPURenderer* renderer, SDL_GPUCommandBuffer* command_buffer,
    SDL_GPURenderPass* render_pass, const void* matrix_2d, const void* matrix_3d)
{
    DebugGroup debug_group(command_buffer, "SDLx_gpu::RenderText3D");
    SDL_BindGPUGraphicsPipeline(render_pass, renderer->text_3d_pipeline);
    SDL_PushGPUVertexUniformData(command_buffer, 0, matrix_3d, 64);
    for (TextInstance3D& instance : renderer->text_instances_3d)
    {
        if (instance.id == NullTextId)
        {
            continue;
        }
        Text& text = renderer->texts[instance.id];
        if (text.passes.empty())
        {
            continue;
        }
        int width;
        int height;
        /* TODO: */
        // TTF_GetTextSize(text.handle, &width, &height);
        // instance.data.x -= width / 2;
        // instance.data.y += height / 2;
        SDL_PushGPUVertexUniformData(command_buffer, 1, &instance.data, sizeof(instance.data));
        SDL_GPUBufferBinding vertex_buffer{};
        SDL_GPUBufferBinding index_buffer{};
        SDL_GPUTextureSamplerBinding atlas_texture{};
        vertex_buffer.buffer = text.vertex_buffer;
        index_buffer.buffer = text.index_buffer;
        atlas_texture.sampler = renderer->nearest_sampler;
        SDL_BindGPUIndexBuffer(render_pass, &index_buffer, SDL_GPU_INDEXELEMENTSIZE_16BIT);
        for (const TextPass& pass : text.passes)
        {
            vertex_buffer.offset = pass.vertex_offset;
            atlas_texture.texture = pass.atlas_texture;
            SDL_BindGPUVertexBuffers(render_pass, 0, &vertex_buffer, 1);
            SDL_BindGPUFragmentSamplers(render_pass, 0, &atlas_texture, 1);
            SDL_DrawGPUIndexedPrimitives(render_pass, pass.num_indices, 1, pass.first_index, 0, 0);
        }
    }
}

static void RenderModels(SDLx_GPURenderer* renderer, SDL_GPUCommandBuffer* command_buffer,
    SDL_GPURenderPass* render_pass, const void* matrix_2d, const void* matrix_3d)
{
    DebugGroup debug_group(command_buffer, "SDLx_gpu::RenderModels");
    /* TODO: reduce pipeline binding */
    for (ModelInstance& instance : renderer->model_instances)
    {
        SDLx_Model* model = renderer->models[instance.data.path][instance.data.type];
        if (!model)
        {
            continue;
        }
        switch (model->type)
        {
        case SDLX_MODELTYPE_VOXOBJ:
            {
                SDL_GPUBufferBinding vertex_buffer{};
                SDL_GPUBufferBinding index_buffer{};
                SDL_GPUTextureSamplerBinding palette_texture{};
                vertex_buffer.buffer = model->vox_obj.vertex_buffer;
                index_buffer.buffer = model->vox_obj.index_buffer;
                palette_texture.sampler = renderer->nearest_sampler;
                palette_texture.texture = model->vox_obj.palette_texture;
                SDL_BindGPUGraphicsPipeline(render_pass, renderer->vox_obj_pipeline);
                SDL_PushGPUVertexUniformData(command_buffer, 0, matrix_3d, 64);
                SDL_PushGPUVertexUniformData(command_buffer, 1, &instance.transform, 64);
                SDL_BindGPUVertexBuffers(render_pass, 0, &vertex_buffer, 1);
                SDL_BindGPUIndexBuffer(render_pass, &index_buffer, model->vox_obj.index_element_size);
                SDL_BindGPUFragmentSamplers(render_pass, 0, &palette_texture, 1);
                SDL_DrawGPUIndexedPrimitives(render_pass, model->vox_obj.num_indices, 1, 0, 0, 0);
            }
            break;
        case SDLX_MODELTYPE_VOXRAW:
            {
                SDL_GPUBufferBinding vertex_buffers[2]{};
                SDL_GPUBufferBinding index_buffer{};
                vertex_buffers[0].buffer = model->vox_raw.vertex_buffer;
                vertex_buffers[1].buffer = model->vox_raw.instance_buffer;
                index_buffer.buffer = model->vox_raw.index_buffer;
                SDL_BindGPUGraphicsPipeline(render_pass, renderer->vox_raw_pipeline);
                SDL_PushGPUVertexUniformData(command_buffer, 0, matrix_3d, 64);
                SDL_PushGPUVertexUniformData(command_buffer, 1, &instance.transform, 64);
                SDL_BindGPUVertexBuffers(render_pass, 0, vertex_buffers, 2);
                SDL_BindGPUIndexBuffer(render_pass, &index_buffer, model->vox_raw.index_element_size);
                SDL_DrawGPUIndexedPrimitives(render_pass, model->vox_raw.num_indices, model->vox_raw.num_instances, 0, 0, 0);
            }
            break;
        default:
            SDL_assert(false);
        }
    }
}

void SDLx_GPUSubmitRenderer(SDLx_GPURenderer* renderer, SDL_GPUCommandBuffer* command_buffer,
    SDL_GPUTexture* color_texture, SDL_GPUTexture* depth_texture, const void* matrix_2d, const void* matrix_3d)
{
    if (!renderer)
    {
        SDL_InvalidParamError("renderer");
        return;
    }
    if (!color_texture)
    {
        SDL_InvalidParamError("color_texture");
        return;
    }
    if (!depth_texture)
    {
        SDL_InvalidParamError("depth_texture");
        return;
    }
    if (!matrix_2d)
    {
        SDL_InvalidParamError("matrix_2d");
        return;
    }
    if (!matrix_3d)
    {
        SDL_InvalidParamError("matrix_3d");
        return;
    }
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);
    if (!copy_pass)
    {
        SDL_Log("Failed to begin copy pass: %s", SDL_GetError());
        return;
    }
    renderer->buffer_2d.Upload(renderer->device, copy_pass);
    renderer->buffer_line_3d.Upload(renderer->device, copy_pass);
    for (TextInstance2D& instance : renderer->text_instances_2d)
    {
        UploadText(renderer, instance.id, copy_pass);
    }
    for (TextInstance3D& instance : renderer->text_instances_3d)
    {
        UploadText(renderer, instance.id, copy_pass);
    }
    for (ModelInstance& instance : renderer->model_instances)
    {
        renderer->model_requests.push_back(instance.data);
    }
    for (ModelData& data : renderer->model_requests)
    {
        auto& models = renderer->models[data.path];
        if (!models[data.type])
        {
            models[data.type] = SDLx_ModelLoad(renderer->device, copy_pass, data.path.data(), data.type);
        }
    }
    SDL_EndGPUCopyPass(copy_pass);
    SDL_GPUColorTargetInfo color_info{};
    color_info.texture = color_texture;
    color_info.load_op = SDL_GPU_LOADOP_LOAD;
    color_info.store_op = SDL_GPU_STOREOP_STORE;
    SDL_GPUDepthStencilTargetInfo depth_info{};
    depth_info.texture = depth_texture;
    depth_info.load_op = SDL_GPU_LOADOP_LOAD;
    depth_info.stencil_load_op = SDL_GPU_LOADOP_LOAD;
    depth_info.store_op = SDL_GPU_STOREOP_STORE;
    SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(command_buffer, &color_info, 1, &depth_info);
    if (!render_pass)
    {
        SDL_Log("Failed to begin render pass: %s", SDL_GetError());
        return;
    }
    RenderShapes3D(renderer, command_buffer, render_pass, matrix_2d, matrix_3d);
    RenderModels(renderer, command_buffer, render_pass, matrix_2d, matrix_3d);
    RenderShapes2D(renderer, command_buffer, render_pass, matrix_2d, matrix_3d);
    RenderText2D(renderer, command_buffer, render_pass, matrix_2d, matrix_3d);
    RenderText3D(renderer, command_buffer, render_pass, matrix_2d, matrix_3d);
    SDL_EndGPURenderPass(render_pass);
    renderer->draw_passes_2d.clear();
    renderer->text_instances_2d.clear();
    renderer->text_instances_3d.clear();
    renderer->model_instances.clear();
}