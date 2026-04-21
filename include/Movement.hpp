#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace Engine
{
    class Movement
    {
    public:
        static inline bool active = false;

        static inline std::vector<int> pointIds;
        static inline std::vector<glm::vec3> startPositions;

        static inline glm::vec3 axisOrigin{0.0f, 0.0f, 0.0f};
        static inline glm::vec3 axisDir{1.0f, 0.0f, 0.0f};

        static inline float startT = 0.0f;

        static void Begin(const std::vector<int>& ids, const glm::vec3& a, const glm::vec3& b);
        static void PreviewTo(const glm::vec3& worldPoint);
        static void Apply();
        static void Cancel();
        static void Reset();

        static bool IsActive() { return active; }

    private:
        static float ProjectT(const glm::vec3& p);
    };
}