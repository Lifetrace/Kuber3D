#include "Movement.hpp"

void Engine::Movement::Init(std::vector<int> points, glm::vec3 Camps[2])
{
    IsEnable = true;

    for (int x : points)
    {
        PointsAbleToMove.push_back(x);
    }

    ToCamp[1] = Camps[1];
    ToCamp[2] = Camps[2];

    Axe = Camps[2] - Camps[1];
}

void Engine::Movement::DestroyMovement()
{
    IsEnable = false;

    PointsAbleToMove.clear();

    Axe = glm::vec3(0.0f, 0.0f, 0.0f);
    ToCamp[1] = glm::vec3(0.0f, 0.0f, 0.0f);
    ToCamp[2] = glm::vec3(0.0f, 0.0f, 0.0f);
}

void Engine::Movement::MoveTo(glm::vec3 A)
{

}