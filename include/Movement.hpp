#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace Engine
{
    class Movement
    {
        static bool IsEnable;

        static std::vector<int> PointsAbleToMove;

        static glm::vec3 Axe;

        static glm::vec3 ToCamp[2];

    public:
        void SetAxe(glm::vec3 Axe)
        {
            this->Axe = glm::normalize(Axe);
        }

        glm::vec3 GetAxe(glm::vec3 Axe)
        {
            return this->Axe;
        }

        void Init(std::vector<int> points, glm::vec3 Camps[2]);

        void MoveTo(glm::vec3 A);

        void DestroyMovement();
    };
}