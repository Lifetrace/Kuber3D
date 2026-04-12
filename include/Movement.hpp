#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace Engine
{
    class Movement
    {
        static inline bool IsEnable;

        static inline std::vector<int> PointsAbleToMove;

        static inline glm::vec3 Axe;

        static inline glm::vec3 ToCamp[2];

    public:
        void SetAxe(glm::vec3 Axe)
        {
            this->Axe = glm::normalize(Axe);
        }

        glm::vec3 GetAxe()
        {
            return this->Axe;
        }

        void Init(std::vector<int> points, glm::vec3 Camps[2]);

        void MoveTo(glm::vec3 A);

        void DestroyMovement();
    };
}