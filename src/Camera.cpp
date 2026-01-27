#include "Camera.hpp"


glm::vec3 Engine::Camera::position() const {
    float cy = cos(yaw), sy = sin(yaw);
    float cp = cos(pitch), sp = sin(pitch);
    glm::vec3 dir = glm::normalize(glm::vec3(cy*cp, sp, sy*cp)); 
    return target - dir * distance;
}

glm::mat4 Engine::Camera::view() const {
    return glm::lookAt(position(), target, glm::vec3(0,1,0));
}

glm::mat4 Engine::Camera::proj(float aspect) const {
    return glm::perspective(glm::radians(fov), aspect, 0.01f, 1000.0f);
}