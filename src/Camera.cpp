#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>

#include "Assert.hpp"
#include "Camera.hpp"

static constexpr float Rotate = 0.002f;
static constexpr float Zoom = 20.0f;

namespace Crobots
{

Camera::Camera()
    : m_type{CameraType::Perspective}
    , m_mode{CameraMode::Arcball}
    , m_matrix3D{}
    , m_view{}
    , m_proj{}
    , m_position{50.0f, 100.0f, -80.0f}
    , m_width{}
    , m_height{}
    , m_fov{glm::radians(60.0f)}
    , m_near{0.1f}
    , m_far{1000.0f}
    , m_pitch{glm::radians(-45.0f)}
    , m_yaw{}
    , m_distance{100.0f} {}

void Camera::Update()
{
    static constexpr glm::vec3 Up{0.0f, 1.0f, 0.0f};
    glm::vec3 center;
    glm::vec3 eye;
    glm::vec3 vector;
    vector.x = std::cos(m_pitch) * std::cos(m_yaw);
    vector.y = std::sin(m_pitch);
    vector.z = std::cos(m_pitch) * std::sin(m_yaw);
    switch (m_mode)
    {
    case CameraMode::Arcball:
        center = m_center;
        eye = m_center - vector * m_distance;
        break;
    default:
        CROBOTS_ASSERT(false);
    }
    m_view = glm::lookAt(eye, center, Up);
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
    m_matrix3D = m_proj * m_view;
    m_matrix2D = glm::ortho(0.0f, w, 0.0f, h);
}

void Camera::Handle(SDL_Event* event)
{
    switch (m_mode)
    {
    case CameraMode::Arcball:
        switch (event->type)
        {
        case SDL_EVENT_MOUSE_MOTION:
            if (event->motion.state & SDL_BUTTON_LMASK)
            {
                static constexpr float Pitch = glm::pi<float>() / 2.0f - 0.01f;
                m_yaw += event->motion.xrel * Rotate;
                m_pitch = std::clamp(m_pitch - event->motion.yrel * Rotate, -Pitch, Pitch);
            }
            break;
        case SDL_EVENT_MOUSE_WHEEL:
            m_distance = std::max(1.0f, m_distance - event->wheel.y * Zoom);
            break;
        }
        break;
    default:
        CROBOTS_ASSERT(false);
    }
}

void Camera::SetType(CameraType type)
{
    m_type = type;
}

void Camera::SetMode(CameraMode mode)
{
    m_mode = mode;
}

void Camera::SetViewport(uint32_t width, uint32_t height)
{
    m_width = width;
    m_height = height;
}

void Camera::SetCenter(float x, float y)
{
    m_center = glm::vec3{x, 0.0f, y};
}

const glm::mat4& Camera::GetMatrix3D() const
{
    return m_matrix3D;
}

const glm::mat4& Camera::GetView() const
{
    return m_view;
}

const glm::mat4& Camera::GetProj() const
{
    return m_proj;
}

const glm::mat4& Camera::GetMatrix2D() const
{
    return m_matrix2D;
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