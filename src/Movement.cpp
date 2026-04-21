#include "Movement.hpp"

#include "Buffers.hpp"

float Engine::Movement::ProjectT(const glm::vec3& p)
{
    return glm::dot(p - axisOrigin, axisDir);
}

void Engine::Movement::PreviewTo(const glm::vec3& worldPoint)
{
    if (!active) return;
    if (pointIds.size() != startPositions.size()) return;

    float currentT = ProjectT(worldPoint);
    float delta = currentT - startT;

    for (int i = 0; i < (int)pointIds.size(); ++i)
    {
        int id = pointIds[i];
        if (id < 0 || id >= (int)Engine::Buffers::positions.size()) continue;

        Engine::Buffers::positions[id] = startPositions[i] + axisDir * delta;
    }

    Engine::Buffers::Update();
}

void Engine::Movement::Begin(const std::vector<int>& ids, const glm::vec3& a, const glm::vec3& b)
{
    Reset();

    if (ids.empty()) return;

    glm::vec3 dir = b - a;
    if (glm::length(dir) < 1e-6f) return;

    active = true;
    pointIds = ids;
    axisOrigin = a;
    axisDir = glm::normalize(dir);

    startPositions.reserve(ids.size());
    for (int id : ids)
    {
        if (id >= 0 && id < (int)Engine::Buffers::positions.size())
            startPositions.push_back(Engine::Buffers::positions[id]);
    }

    if (!startPositions.empty())
        startT = ProjectT(startPositions[0]);
}

void Engine::Movement::Cancel()
{
    if (!active) return;
    if (pointIds.size() == startPositions.size())
    {
        for (int i = 0; i < (int)pointIds.size(); ++i)
        {
            int id = pointIds[i];
            if (id >= 0 && id < (int)Engine::Buffers::positions.size())
                Engine::Buffers::positions[id] = startPositions[i];
        }

        Engine::Buffers::Update();
    }

    Reset();
}

void Engine::Movement::Apply()
{
    Reset();
}

void Engine::Movement::Reset()
{
    active = false;
    pointIds.clear();
    startPositions.clear();
    axisOrigin = glm::vec3(0.0f);
    axisDir = glm::vec3(1.0f, 0.0f, 0.0f);
    startT = 0.0f;
}