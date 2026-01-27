#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace Engine{
    struct Cross{
        static float Cross2(const glm::vec2& a, const glm::vec2& b){
            return a.x*b.y - a.y*b.x;
        }
        static glm::vec2 XZ(const glm::vec3& p){ return {p.x, p.z}; }

        static bool RaySeg2D(const glm::vec2& O, const glm::vec2& d, const glm::vec2& A, const glm::vec2& B, float& outT, float& outU){
            glm::vec2 s = B - A;
            float denom = Cross2(d, s);
            if (std::abs(denom) < 1e-7f) return false;

            glm::vec2 AO = A - O;
            float t = Cross2(AO, s) / denom;
            float u = Cross2(AO, d) / denom;

            if (t < 1e-4f) return false;
            if (u < -1e-4f || u > 1.0f + 1e-4f) return false;

            outT = t;
            outU = u;
            return true;
        }
    };
}