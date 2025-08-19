#pragma once

#include <glm/glm.hpp>

namespace Crobots
{

enum class CameraType
{
    Perspective,
    Ortho,
};

class Camera
{
public:
    Camera();
    void Update();
    void SetType(CameraType type);
    void SetPosition(const glm::vec3& position);
    void SetDirection(const glm::vec3& direction);
    void SetViewport(uint32_t width, uint32_t height);
    const glm::mat4& GetViewProj() const;
    const glm::mat4& GetView() const;
    const glm::mat4& GetProj() const;
    const glm::mat4& GetOrtho() const;
    uint32_t GetWidth() const;
    uint32_t GetHeight() const;

private:
    CameraType m_type;
    glm::mat4 m_viewProj;
    glm::mat4 m_view;
    glm::mat4 m_proj;
    glm::mat4 m_ortho;
    glm::vec3 m_position;
    glm::vec3 m_direction;
    uint32_t m_width;
    uint32_t m_height;
    float m_fov;
    float m_near;
    float m_far;
};

}