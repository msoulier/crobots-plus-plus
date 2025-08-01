#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Assert.hpp"
#include "Camera.hpp"

namespace Crobots
{

Camera::Camera()
    : m_type{CameraType::Ortho}
    , m_viewProj{}
    , m_view{}
    , m_proj{}
    , m_position{}
    , m_direction{0.0f, 0.0f, 1.0f}
    , m_viewport{}
    , m_fov{glm::radians(60.0f)}
    , m_near{0.1f}
    , m_far{1000.0f} {}

void Camera::Update()
{
    static constexpr glm::vec3 Up{0.0f, 1.0f, 0.0f};
    m_view = glm::lookAt(m_position, m_position + m_direction, Up);
    float w = m_viewport.x;
    float h = m_viewport.y;
    switch (m_type)
    {
    case CameraType::Perspective:
        m_proj = glm::perspective(m_fov, w / h, m_near, m_far);
        break;
    case CameraType::Ortho:
        m_proj = glm::ortho(-w / 2.0f, w / 2.0f, h / 2.0f, -h / 2.0f);
        break;
    default:
        CROBOTS_ASSERT(false);
    }
    m_viewProj = m_proj * m_view;
}

void Camera::SetType(CameraType type)
{
    m_type = type;
}

void Camera::SetPosition(const glm::vec3& position)
{
    m_position = position;
}

void Camera::SetDirection(const glm::vec3& direction)
{
    m_direction = direction;
}

void Camera::SetViewport(const glm::vec2& viewport)
{
    m_viewport = viewport;
}

void Camera::SetDirectionFromYaw(float yaw)
{
    m_direction.x = glm::cos(yaw);
    m_direction.y = 0.0f;
    m_direction.z = glm::sin(yaw);
    m_direction = glm::normalize(m_direction);
}

const glm::mat4& Camera::GetViewProj() const
{
    return m_viewProj;
}

const glm::mat4& Camera::GetView() const
{
    return m_view;
}

const glm::mat4& Camera::GetProj() const
{
    return m_proj;
}

}