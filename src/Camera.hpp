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
    void SetViewport(const glm::vec2& viewport);
    void SetDirectionFromYaw(float yaw);
    const glm::mat4& GetViewProj() const;
    const glm::mat4& GetView() const;
    const glm::mat4& GetProj() const;

private:
    CameraType m_type;
    glm::mat4 m_viewProj;
    glm::mat4 m_view;
    glm::mat4 m_proj;
    glm::vec3 m_position;
    glm::vec3 m_direction;
    glm::vec2 m_viewport;
    float m_fov;
    float m_near;
    float m_far;
};

}