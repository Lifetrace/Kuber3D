#pragma once

#include <glm/vec3.hpp>

namespace Engine
{
    class Operations
    {
    public:
        static void CutLine(float p, float q);
        static bool ExtendUsingCutLine();
        static void PerpToPlane();
    };
}