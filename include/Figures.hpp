#pragma once

namespace Engine
{
    class Figures
    {
    public:
        static void CreateCube();
        static void CreateCuboid();
        static void CreatePyramid();
        static void CreateTetrahedron();
        static void CreateCircle(int N, float R, float cx, float cy, float cz); // in dev
    };
}