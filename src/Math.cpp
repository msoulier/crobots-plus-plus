#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Math.hpp"

namespace Crobots
{

glm::mat4 CreateModelMatrix(const glm::vec3& position, const glm::vec3& rotation)
{
    /* scale * rotation * translation */
    static constexpr glm::vec3 Scale{1.0f, 1.0f, 1.0f};
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, rotation.x, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, rotation.y, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, Scale);
    return model;
}

}