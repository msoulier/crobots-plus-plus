#include <SDL3/SDL.h>
#include <nlohmann/json.hpp>

#include <cstdint>
#include <exception>
#include <format>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>

#include "Assert.hpp"
#include "Log.hpp"
#include "Shader.hpp"

namespace Crobots
{

static void* Load(SDL_GPUDevice* device, const std::string_view& name)
{
    SDL_GPUShaderFormat shaderFormat = SDL_GetGPUShaderFormats(device);
    const char* entrypoint;
    const char* fileExtension;
    if (shaderFormat & SDL_GPU_SHADERFORMAT_SPIRV)
    {
        shaderFormat = SDL_GPU_SHADERFORMAT_SPIRV;
        entrypoint = "main";
        fileExtension = "spv";
    }
    else if (shaderFormat & SDL_GPU_SHADERFORMAT_DXIL)
    {
        shaderFormat = SDL_GPU_SHADERFORMAT_DXIL;
        entrypoint = "main";
        fileExtension = "dxil";
    }
    else if (shaderFormat & SDL_GPU_SHADERFORMAT_MSL)
    {
        shaderFormat = SDL_GPU_SHADERFORMAT_MSL;
        entrypoint = "main0";
        fileExtension = "msl";
    }
    else
    {
        CROBOTS_ASSERT(false);
    }
    std::string shaderPath = std::format("{}.{}", name, fileExtension);
    std::ifstream shaderFile(shaderPath, std::ios::binary);
    if (shaderFile.fail())
    {
        CROBOTS_LOG("Failed to open shader: %s", shaderPath.data());
        return nullptr;
    }
    std::string jsonPath = std::format("{}.json", name);
    std::ifstream jsonFile(jsonPath, std::ios::binary);
    if (jsonFile.fail())
    {
        CROBOTS_LOG("Failed to open json: %s", jsonPath.data());
        return nullptr;
    }
    std::string shaderData(std::istreambuf_iterator<char>(shaderFile), {});
    nlohmann::json json;
    try
    {
        jsonFile >> json;
    }
    catch (const std::exception& exception)
    {
        CROBOTS_LOG("Failed to parse json: %s, %s", jsonPath.data(), exception.what());
        return nullptr;
    }
    void* shader = nullptr;
    if (name.contains(".comp"))
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
        info.code = reinterpret_cast<Uint8*>(shaderData.data());
        info.code_size = shaderData.size();
        info.entrypoint = entrypoint;
        info.format = shaderFormat;
        shader = SDL_CreateGPUComputePipeline(device, &info);
    }
    else
    {
        SDL_GPUShaderCreateInfo info{};
        info.num_samplers = json["samplers"];
        info.num_storage_textures = json["storage_textures"];
        info.num_storage_buffers = json["storage_buffers"];
        info.num_uniform_buffers = json["uniform_buffers"];
        info.code = reinterpret_cast<Uint8*>(shaderData.data());
        info.code_size = shaderData.size();
        info.entrypoint = entrypoint;
        info.format = shaderFormat;
        if (name.contains(".frag"))
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
        CROBOTS_LOG("Failed to create shader: %s, %s", name.data(), SDL_GetError());
        return nullptr;
    }
    return shader;
}

SDL_GPUShader* LoadShader(SDL_GPUDevice* device, const std::string_view& name)
{
    return static_cast<SDL_GPUShader*>(Load(device, name));
}

SDL_GPUComputePipeline* LoadComputePipeline(SDL_GPUDevice* device, const std::string_view& name)
{
    return static_cast<SDL_GPUComputePipeline*>(Load(device, name));
}

}