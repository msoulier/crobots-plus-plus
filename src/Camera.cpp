#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Assert.hpp"
#include "Camera.hpp"

namespace Crobots
{

Camera::Camera()
    : m_type{CameraType::Perspective}
    , m_viewProj{}
    , m_view{}
    , m_proj{}
    , m_position{50.0f, 100.0f, -80.0f}
    /* TODO: convert to pitch. honestly scrap the whole camera and support arcball and freecam */
    , m_direction{glm::normalize(glm::vec3(0.0f, -1.0f, 1.0f))}
    , m_width{}
    , m_height{}
    , m_fov{glm::radians(60.0f)}
    , m_near{0.1f}
    , m_far{1000.0f} {}

void Camera::Update()
{
    static constexpr glm::vec3 Up{0.0f, 1.0f, 0.0f};
    m_view = glm::lookAt(m_position, m_position + m_direction, Up);
    float w = m_width;
    float h = m_height;
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
    m_ortho = glm::ortho(0.0f, w, 0.0f, h);
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

void Camera::SetViewport(uint32_t width, uint32_t height)
{
    m_width = width;
    m_height = height;
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

const glm::mat4& Camera::GetOrtho() const
{
    return m_ortho;
}

uint32_t Camera::GetWidth() const
{
    return m_width;
}

uint32_t Camera::GetHeight() const
{
    return m_height;
}

}