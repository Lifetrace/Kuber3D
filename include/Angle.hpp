#pragma once

#include <vector>
#include <glm/vec3.hpp>

namespace Engine
{
    struct Angle
    {
        int idxA = -1;
        int idxB = -1;
        int idxC = -1;

        bool active = false;
        std::vector<glm::vec3> arcPoints;

        float radius = 0.8f;

        Angle() = default;

        Angle(int a, int b, int c, bool isActive = true)
            : idxA(a), idxB(b), idxC(c), active(isActive)
        {
        }

        ~Angle() = default;

        bool IsValid() const
        {
            return idxA >= 0 && idxB >= 0 && idxC >= 0;
        }
    };

    namespace AngleUtils
    {
        bool BuildArcPoints(
            const glm::vec3& A,
            const glm::vec3& B,
            const glm::vec3& C,
            float radius,
            std::vector<glm::vec3>& outArc,
            int segments = 32);

        bool RebuildAngleArc(
            Angle& angle,
            const std::vector<glm::vec3>& positions,
            int segments = 32);
    }
}