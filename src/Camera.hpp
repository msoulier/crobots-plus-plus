#pragma once

#include <SDL3/SDL.h>
#include <glm/glm.hpp>

#include <memory>

namespace Crobots
{

class IRobot;

enum class CameraType
{
    Perspective,
    Ortho,
};

enum class CameraMode
{
    Arcball,
};

class Camera
{
public:
    Camera();
    void Update();
    void Handle(SDL_Event* event);
    void SetType(CameraType type);
    void SetMode(CameraMode mode);
    void SetViewport(uint32_t width, uint32_t height);
    void SetCenter(float x, float y);
    const glm::mat4& GetMatrix3D() const;
    const glm::mat4& GetView() const;
    const glm::mat4& GetProj() const;
    const glm::mat4& GetMatrix2D() const;
    uint32_t GetWidth() const;
    uint32_t GetHeight() const;

private:
    CameraType m_type;
    CameraMode m_mode;
    glm::mat4 m_matrix3D;
    glm::mat4 m_view;
    glm::mat4 m_proj;
    glm::mat4 m_matrix2D;
    glm::vec3 m_position;
    glm::vec3 m_center;
    uint32_t m_width;
    uint32_t m_height;
    float m_fov;
    float m_near;
    float m_far;
    float m_pitch;
    float m_yaw;
    float m_distance;
};

}