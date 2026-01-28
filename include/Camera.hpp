#pragma once
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

namespace Engine{
    struct Camera {
    glm::vec3 target = {0,0,0};
    float distance = 6.0f;   
    float yaw = -2.14159265f;          
    float pitch = -1.0f;       
    float fov = 60.0f;

    glm::vec3 position() const;

    glm::mat4 view() const;

    glm::mat4 proj(float aspect) const;
};
}
