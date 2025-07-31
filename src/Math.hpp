#pragma once

#include <glm/glm.hpp>

namespace Crobots
{

glm::mat4 CreateModelMatrix(const glm::vec3& position, const glm::vec3& rotation);

}