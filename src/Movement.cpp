#include "Movement.hpp"

#include "Buffers.hpp"

void Engine::Movement::Init(std::vector<int> points, glm::vec3 Camps[2])
{
    IsEnable = true;

    for (int x : points)
    {
        PointsAbleToMove.push_back(x);
    }

    ToCamp[0] = Camps[0];
    ToCamp[1] = Camps[1];

    Axe = Camps[1] - Camps[0];
}

void Engine::Movement::DestroyMovement()
{
    IsEnable = false;

    PointsAbleToMove.clear();

    Axe = glm::vec3(0.0f, 0.0f, 0.0f);
    ToCamp[0] = glm::vec3(0.0f, 0.0f, 0.0f);
    ToCamp[1] = glm::vec3(0.0f, 0.0f, 0.0f);
}

void Engine::Movement::MoveTo(glm::vec3 A)
{
    
}