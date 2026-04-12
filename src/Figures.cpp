#include "Figures.hpp"

#include "Buffers.hpp"

#include <cmath>

// Standard points colors
float pr = Engine::Buffers::pr;
float pg = Engine::Buffers::pg;
float pb = Engine::Buffers::pb;
float pa = Engine::Buffers::pa;

void Engine::Figures::CreateCuboid()
{
    Engine::Buffers::AddPoint(-1.0f, 0.0f, -1.8f, pr, pg, pb, pa); // 0
    Engine::Buffers::AddPoint(1.0f, 0.0f, -1.8f, pr, pg, pb, pa);  // 1
    Engine::Buffers::AddPoint(-1.0f, 0.0f, 1.8f, pr, pg, pb, pa);  // 2
    Engine::Buffers::AddPoint(1.0f, 0.0f, 1.8f, pr, pg, pb, pa);   // 3

    Engine::Buffers::AddPoint(-1.0f, 2.0f, -1.8f, pr, pg, pb, pa); // 4
    Engine::Buffers::AddPoint(1.0f, 2.0f, -1.8f, pr, pg, pb, pa);  // 5
    Engine::Buffers::AddPoint(-1.0f, 2.0f, 1.8f, pr, pg, pb, pa);  // 6
    Engine::Buffers::AddPoint(1.0f, 2.0f, 1.8f, pr, pg, pb, pa);   // 7

    // 0
    Engine::Buffers::ConnectPointsLine(0, 1);
    Engine::Buffers::ConnectPointsLine(0, 2);
    Engine::Buffers::ConnectPointsLine(0, 4);
    // 1
    Engine::Buffers::ConnectPointsLine(1, 5);
    Engine::Buffers::ConnectPointsLine(1, 3);
    // 3
    Engine::Buffers::ConnectPointsLine(3, 2);
    Engine::Buffers::ConnectPointsLine(3, 7);
    // 2
    Engine::Buffers::ConnectPointsLine(2, 6);
    // 4
    Engine::Buffers::ConnectPointsLine(4, 5);
    Engine::Buffers::ConnectPointsLine(4, 6);
    // 5
    Engine::Buffers::ConnectPointsLine(5, 7);
    // 7
    Engine::Buffers::ConnectPointsLine(7, 6);

    Engine::Buffers::AddQuad(0, 1, 3, 2);

    Engine::Buffers::AddQuad(4, 6, 7, 5);

    Engine::Buffers::AddQuad(2, 3, 7, 6);

    Engine::Buffers::AddQuad(0, 4, 5, 1);

    Engine::Buffers::AddQuad(0, 2, 6, 4);

    Engine::Buffers::AddQuad(1, 5, 7, 3);

    Engine::Buffers::Update();
}

void Engine::Figures::CreateTetrahedron()
{
    Engine::Buffers::AddPoint(-1.0f, 0.0f, -0.8f, pr, pg, pb, pa); // 0
    Engine::Buffers::AddPoint(1.0f, 0.0f, -0.8f, pr, pg, pb, pa);  // 1
    Engine::Buffers::AddPoint(0.0f, 0.0f, 1.3f, pr, pg, pb, pa);   // 2

    Engine::Buffers::AddPoint(0.0f, 2.0f, 0.0f, pr, pg, pb, pa); // 3

    // 0
    Engine::Buffers::ConnectPointsLine(0, 1);
    Engine::Buffers::ConnectPointsLine(0, 2);
    Engine::Buffers::ConnectPointsLine(0, 3);
    // 1
    Engine::Buffers::ConnectPointsLine(1, 2);
    Engine::Buffers::ConnectPointsLine(1, 3);
    // 3
    Engine::Buffers::ConnectPointsLine(3, 2);

    Engine::Buffers::AddFace(0, 1, 2);

    Engine::Buffers::AddFace(0, 1, 3);

    Engine::Buffers::AddFace(1, 2, 3);

    Engine::Buffers::AddFace(2, 0, 3);

    Engine::Buffers::Update();
}

void Engine::Figures::CreatePyramid()
{
    Engine::Buffers::AddPoint(-1.0f, 0.0f, -1.0f, pr, pg, pb, pa); // 0
    Engine::Buffers::AddPoint(1.0f, 0.0f, -1.0f, pr, pg, pb, pa);  // 1
    Engine::Buffers::AddPoint(-1.0f, 0.0f, 1.0f, pr, pg, pb, pa);  // 2
    Engine::Buffers::AddPoint(1.0f, 0.0f, 1.0f, pr, pg, pb, pa);   // 3

    Engine::Buffers::AddPoint(0.0f, 2.0f, 0.0f, pr, pg, pb, pa); // 4

    // 0
    Engine::Buffers::ConnectPointsLine(0, 1);
    Engine::Buffers::ConnectPointsLine(0, 2);
    Engine::Buffers::ConnectPointsLine(0, 4);
    // 1
    Engine::Buffers::ConnectPointsLine(1, 4);
    Engine::Buffers::ConnectPointsLine(1, 3);
    // 3
    Engine::Buffers::ConnectPointsLine(3, 2);
    Engine::Buffers::ConnectPointsLine(3, 4);
    // 2
    Engine::Buffers::ConnectPointsLine(2, 4);

    Engine::Buffers::AddQuad(0, 1, 3, 2);

    Engine::Buffers::AddFace(0, 1, 4);

    Engine::Buffers::AddFace(1, 3, 4);

    Engine::Buffers::AddFace(3, 2, 4);

    Engine::Buffers::AddFace(2, 0, 4);

    Engine::Buffers::Update();
}

void Engine::Figures::CreateCube()
{
    Engine::Buffers::AddPoint(-1.0f, 0.0f, -1.0f, pr, pg, pb, pa); // 0
    Engine::Buffers::AddPoint(1.0f, 0.0f, -1.0f, pr, pg, pb, pa);  // 1
    Engine::Buffers::AddPoint(-1.0f, 0.0f, 1.0f, pr, pg, pb, pa);  // 2
    Engine::Buffers::AddPoint(1.0f, 0.0f, 1.0f, pr, pg, pb, pa);   // 3

    Engine::Buffers::AddPoint(-1.0f, 2.0f, -1.0f, pr, pg, pb, pa); // 4
    Engine::Buffers::AddPoint(1.0f, 2.0f, -1.0f, pr, pg, pb, pa);  // 5
    Engine::Buffers::AddPoint(-1.0f, 2.0f, 1.0f, pr, pg, pb, pa);  // 6
    Engine::Buffers::AddPoint(1.0f, 2.0f, 1.0f, pr, pg, pb, pa);   // 7

    // 0
    Engine::Buffers::ConnectPointsLine(0, 1);
    Engine::Buffers::ConnectPointsLine(0, 2);
    Engine::Buffers::ConnectPointsLine(0, 4);
    // 1
    Engine::Buffers::ConnectPointsLine(1, 5);
    Engine::Buffers::ConnectPointsLine(1, 3);
    // 3
    Engine::Buffers::ConnectPointsLine(3, 2);
    Engine::Buffers::ConnectPointsLine(3, 7);
    // 2
    Engine::Buffers::ConnectPointsLine(2, 6);
    // 4
    Engine::Buffers::ConnectPointsLine(4, 5);
    Engine::Buffers::ConnectPointsLine(4, 6);
    // 5
    Engine::Buffers::ConnectPointsLine(5, 7);
    // 7
    Engine::Buffers::ConnectPointsLine(7, 6);

    Engine::Buffers::AddQuad(0, 1, 3, 2);

    Engine::Buffers::AddQuad(4, 6, 7, 5);

    Engine::Buffers::AddQuad(2, 3, 7, 6);

    Engine::Buffers::AddQuad(0, 4, 5, 1);

    Engine::Buffers::AddQuad(0, 2, 6, 4);

    Engine::Buffers::AddQuad(1, 5, 7, 3);

    Engine::Buffers::Update();
}

void Engine::Figures::CreateCircle(int N, float R, float cx, float cy, float cz)
{
    const float TWO_PI = 6.283185307179586f;

    for (int i = 0; i < N; ++i)
    {
        float t = TWO_PI * (float)i / (float)N;

        float x = cx + R * std::sin(t);
        float y = cy;
        float z = cz + R * std::cos(t);

        Engine::Buffers::AddPoint(x, y, z, pr, pg, pb, pa);

        for (int pt_b = 0; pt_b < N; pt_b++)
        {
            Engine::Buffers::ConnectPointsLine(pt_b, pt_b + 1);
            Engine::Buffers::ConnectPointsLine(N - 1, 0);
        }
    }
}
