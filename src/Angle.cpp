#include "Angle.hpp"

#include <algorithm>
#include <cmath>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>

namespace Engine::AngleUtils
{
    bool BuildArcPoints(
        const glm::vec3& A,
        const glm::vec3& B,
        const glm::vec3& C,
        float radius,
        std::vector<glm::vec3>& outArc,
        int segments)
    {
        outArc.clear();

        glm::vec3 u = A - B;
        glm::vec3 v = C - B;

        if (glm::length2(u) < 1e-8f || glm::length2(v) < 1e-8f)
            return false;

        u = glm::normalize(u);
        v = glm::normalize(v);

        glm::vec3 n = glm::cross(u, v);
        if (glm::length2(n) < 1e-8f)
            return false;

        n = glm::normalize(n);

        float d = glm::clamp(glm::dot(u, v), -1.0f, 1.0f);
        float totalAngle = std::acos(d);

        segments = std::max(8, segments);
        outArc.reserve((size_t)segments + 1);

        for (int i = 0; i <= segments; ++i)
        {
            float t = (float)i / (float)segments;
            float ang = totalAngle * t;

            glm::mat4 R(1.0f);
            R = glm::rotate(R, ang, n);

            glm::vec3 dir = glm::normalize(glm::vec3(R * glm::vec4(u, 0.0f)));
            outArc.push_back(B + dir * radius);
        }

        return true;
    }

    bool RebuildAngleArc(
        Angle& angle,
        const std::vector<glm::vec3>& positions,
        int segments)
    {
        if (!angle.active || !angle.IsValid())
            return false;

        if (angle.idxA >= (int)positions.size() ||
            angle.idxB >= (int)positions.size() ||
            angle.idxC >= (int)positions.size())
        {
            angle.arcPoints.clear();
            return false;
        }

        const glm::vec3& A = positions[angle.idxA];
        const glm::vec3& B = positions[angle.idxB];
        const glm::vec3& C = positions[angle.idxC];

        return BuildArcPoints(A, B, C, angle.radius, angle.arcPoints, segments);
    }
}