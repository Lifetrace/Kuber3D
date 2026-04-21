#include "VectorMath.hpp"

#include "Ray.hpp"

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

#include <algorithm>

void Engine::VectorMath::SortUnique(std::vector<int> &v)
{
    std::sort(v.begin(), v.end());
    v.erase(std::unique(v.begin(), v.end()), v.end());
}

int Engine::VectorMath::PickPointByPixels(const std::vector<glm::vec3> &positions, double mx, double my, const glm::mat4 &view, const glm::mat4 &proj, int w, int h, float radiusPx)
{
    int best = -1;
    float bestD2 = radiusPx * radiusPx;

    glm::vec2 m((float)mx, (float)my);

    for (int i = 0; i < (int)positions.size(); i++)
    {
        glm::vec2 p;
        if (!Engine::WorldToScreen(positions[i], view, proj, w, h, p))
            continue;

        glm::vec2 d = p - m;
        float d2 = d.x * d.x + d.y * d.y;
        if (d2 <= bestD2)
        {
            bestD2 = d2;
            best = i;
        }
    }
    return best;
}

bool Engine::VectorMath::RayIntersectsSegment3D(const glm::vec3 &A, const glm::vec3 &B, const glm::vec3 &C, const glm::vec3 &D, glm::vec3 &outHit, float eps)
{
    glm::vec3 R = B - A;
    glm::vec3 S = D - C;
    glm::vec3 W0 = A - C;

    float a = glm::dot(R, R);
    float b = glm::dot(R, S);
    float c = glm::dot(S, S);
    float d = glm::dot(R, W0);
    float e = glm::dot(S, W0);

    if (a <= eps)
    {
        glm::vec3 v = A - C;
        float ss = c;

        if (ss <= eps)
        {

            if (glm::length2(A - C) <= eps * eps)
            {
                outHit = A;
                return true;
            }

            return false;
        }

        float u = glm::dot(v, S) / ss;
        if (u < 0.0f || u > 1.0f)
            return false;

        glm::vec3 Q = C + u * S;

        if (glm::length2(Q - A) <= eps * eps)
        {
            outHit = A;
            return true;
        }
        return false;
    }

    float den = a * c - b * b;

    if (glm::abs(den) <= eps)
    {

        glm::vec3 crossRC = glm::cross((C - A), R);
        if (glm::length2(crossRC) > (eps * eps) * a)
        {
            return false;
        }

        float tC = glm::dot(C - A, R) / a;
        float tD = glm::dot(D - A, R) / a;

        float tMin = glm::min(tC, tD);
        float tMax = glm::max(tC, tD);

        if (tMax < 0.0f)
            return false;

        float tHit = (tMin >= 0.0f) ? tMin : 0.0f;

        if (tHit < tMin - eps || tHit > tMax + eps)
            return false;

        outHit = A + tHit * R;
        return true;
    }

    float t = (b * e - c * d) / den;
    float u = (a * e - b * d) / den;

    if (t < 0.0f)
        return false;
    if (u < 0.0f || u > 1.0f)
        return false;

    glm::vec3 P = A + t * R;
    glm::vec3 Q = C + u * S;

    if (glm::length2(P - Q) > eps * eps)
        return false;

    outHit = (P + Q) * 0.5f;
    return true;
}