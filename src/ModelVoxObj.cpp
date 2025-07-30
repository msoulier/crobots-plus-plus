#include <SDL3/SDL.h>
#include <tiny_obj_loader.h>

#include <cmath>
#include <cstdint>
#include <format>
#include <limits>
#include <string>
#include <string_view>
#include <unordered_map>

#include "Assert.hpp"
#include "Log.hpp"
#include "Model.hpp"
#include "ModelVoxObj.hpp"
#include "Texture.hpp"

namespace Crobots
{

/*
 * LSB to MSB
 * 00-05: x magnitude (6 bits)
 * 06-06: x direction (1 bits)
 * 07-12: y magnitude (6 bits)
 * 13-13: y direction (1 bits)
 * 14-19: z magnitude (6 bits)
 * 20-20: z direction (1 bits)
 * 21-23: normal (3 bits)
 * 24-31: x texcoord (8 bits)
 */
using Vertex = uint32_t;

static Vertex CreateVertex(const tinyobj::attrib_t& attrib, const tinyobj::index_t& index)
{
    static constexpr int PositionScale = 10;
    static constexpr int TexcoordScale = 256;
    int positionX = attrib.vertices[index.vertex_index * 3 + 0] * PositionScale;
    int positionY = attrib.vertices[index.vertex_index * 3 + 1] * PositionScale;
    int positionZ = attrib.vertices[index.vertex_index * 3 + 2] * PositionScale;
    int normalX = attrib.normals[index.normal_index * 3 + 0];
    int normalY = attrib.normals[index.normal_index * 3 + 1];
    int normalZ = attrib.normals[index.normal_index * 3 + 2];
    int texcoordX = attrib.texcoords[index.texcoord_index * 2 + 0] * TexcoordScale;
    uint32_t magnitudeX = std::abs(positionX);
    uint32_t directionX = positionX < 0 ? 1 : 0;
    uint32_t magnitudeY = std::abs(positionY);
    uint32_t directionY = positionY < 0 ? 1 : 0;
    uint32_t magnitudeZ = std::abs(positionZ);
    uint32_t directionZ = positionZ < 0 ? 1 : 0;
    uint32_t normal;
    if (normalX < 0)
    {
        normal = 0;
    }
    else if (normalX > 0)
    {
        normal = 1;
    }
    else if (normalY < 0)
    {
        normal = 2;
    }
    else if (normalY > 0)
    {
        normal = 3;
    }
    else if (normalZ < 0)
    {
        normal = 4;
    }
    else if (normalZ > 0)
    {
        normal = 5;
    }
    else
    {
        CROBOTS_ASSERT(false);
    }
    CROBOTS_ASSERT(magnitudeX < 64);
    CROBOTS_ASSERT(magnitudeY < 64);
    CROBOTS_ASSERT(magnitudeZ < 64);
    CROBOTS_ASSERT(texcoordX < 256);
    Vertex vertex{};
    vertex |= (magnitudeX & 0x3F) << 0;
    vertex |= (directionX & 0x01) << 6;
    vertex |= (magnitudeY & 0x3F) << 7;
    vertex |= (directionY & 0x01) << 13;
    vertex |= (magnitudeZ & 0x3F) << 14;
    vertex |= (directionZ & 0x01) << 20;
    vertex |= (normal & 0x07) << 21;
    vertex |= (texcoordX & 0xFF) << 24;
    return vertex;
}

bool ModelVoxObj::Load(SDL_GPUDevice* device, SDL_GPUCopyPass* copyPass, const std::string_view& name)
{
    std::string objPath = std::format("{}.obj", name);
    std::string pngPath = std::format("{}.png", name);
    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(objPath))
    {
        CROBOTS_LOG("Failed to parse obj: %s", name.data());
        return false;
    }
    const tinyobj::attrib_t& attrib = reader.GetAttrib();
    const tinyobj::shape_t& shape = reader.GetShapes()[0];
    uint32_t maxIndexCount = shape.mesh.num_face_vertices.size() * 3;
    CROBOTS_ASSERT(maxIndexCount <= std::numeric_limits<uint16_t>::max());
    SDL_GPUTransferBuffer* vertexTransferBuffer;
    SDL_GPUTransferBuffer* indexTransferBuffer;
    {
        SDL_GPUTransferBufferCreateInfo info{};
        info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        info.size = maxIndexCount * sizeof(Vertex);
        vertexTransferBuffer = SDL_CreateGPUTransferBuffer(device, &info);
        info.size = maxIndexCount * sizeof(uint16_t);
        indexTransferBuffer = SDL_CreateGPUTransferBuffer(device, &info);
        if (!vertexTransferBuffer || !indexTransferBuffer)
        {
            CROBOTS_LOG("Failed to create transfer buffer(s): %s, %s", name.data(), SDL_GetError());
            return false;
        }
    }
    Vertex* vertexData = static_cast<Vertex*>(SDL_MapGPUTransferBuffer(device, vertexTransferBuffer, false));
    uint16_t* indexData = static_cast<uint16_t*>(SDL_MapGPUTransferBuffer(device, indexTransferBuffer, false));
    if (!vertexData || !indexData)
    {
        CROBOTS_LOG("Failed to map transfer buffer(s): %s, %s", name.data(), SDL_GetError());
        return false;
    }
    uint32_t vertexCount = 0;
    m_indexCount = 0;
    std::unordered_map<Vertex, uint16_t> vertexToIndex;
    for (uint16_t i = 0; i < maxIndexCount; i++)
    {
        tinyobj::index_t index = shape.mesh.indices[i];
        Vertex vertex = CreateVertex(attrib, index);
        auto [it, inserted] = vertexToIndex.try_emplace(vertex, m_indexCount);
        if (inserted)
        {
            vertexData[vertexCount] = vertex;
            indexData[m_indexCount++] = vertexCount++;
        }
        else
        {
            indexData[m_indexCount++] = it->second;
        }
    }
    SDL_UnmapGPUTransferBuffer(device, vertexTransferBuffer);
    SDL_UnmapGPUTransferBuffer(device, indexTransferBuffer);
    {
        SDL_GPUBufferCreateInfo info{};
        info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
        info.size = vertexCount * sizeof(Vertex);
        m_vertexBuffer = SDL_CreateGPUBuffer(device, &info);
        info.usage = SDL_GPU_BUFFERUSAGE_INDEX;
        info.size = m_indexCount * sizeof(uint16_t);
        m_indexBuffer = SDL_CreateGPUBuffer(device, &info);
        if (!m_vertexBuffer || !m_indexBuffer)
        {
            CROBOTS_LOG("Failed to create buffer(s): %s, %s", name.data(), SDL_GetError());
            return false;
        }
    }
    {
        SDL_GPUTransferBufferLocation location{};
        SDL_GPUBufferRegion region{};
        location.transfer_buffer = vertexTransferBuffer;
        region.buffer = m_vertexBuffer;
        region.size = vertexCount * sizeof(Vertex);
        SDL_UploadToGPUBuffer(copyPass, &location, &region, false);
        location.transfer_buffer = indexTransferBuffer;
        region.buffer = m_indexBuffer;
        region.size = m_indexCount * sizeof(uint16_t);
        SDL_UploadToGPUBuffer(copyPass, &location, &region, false);
    }
    SDL_ReleaseGPUTransferBuffer(device, vertexTransferBuffer);
    SDL_ReleaseGPUTransferBuffer(device, indexTransferBuffer);
    m_paletteTexture = LoadTexture(device, copyPass, pngPath);
    if (!m_paletteTexture)
    {
        CROBOTS_LOG("Failed to load texture: %s", name.data());
        return false;
    }
    return true;
}

void ModelVoxObj::Destroy(SDL_GPUDevice* device)
{
    SDL_ReleaseGPUBuffer(device, m_vertexBuffer);
    SDL_ReleaseGPUBuffer(device, m_indexBuffer);
    SDL_ReleaseGPUTexture(device, m_paletteTexture);
    m_vertexBuffer = nullptr;
    m_indexBuffer = nullptr;
    m_paletteTexture = nullptr;
}

}