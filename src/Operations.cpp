#include "Operations.hpp"

#include "Buffers.hpp"
#include "Selecting.hpp"
#include "VectorMath.hpp"

#include "PointActions.hpp"

void Engine::Operations::CutLine(float p, float q)
{
    if (Engine::Selecting::selectedOrder.size() < 2)
        return;
    if (q == 0)
        return;

    Engine::Buffers::DisConnectPointsLine(Engine::Selecting::selectedOrder[0], Engine::Selecting::selectedOrder[1]);

    const int a = Engine::Selecting::selectedOrder[0];
    const int b = Engine::Selecting::selectedOrder[1];

    Engine::Buffers::DisConnectPointsLine(a, b);

    const glm::vec3 A = Engine::Buffers::positions[a];
    const glm::vec3 B = Engine::Buffers::positions[b];

    const glm::vec3 R = A + (B - A) * (p / (p + q));

    Engine::PointActions::RequestPointCreate(R, Engine::PointActions::PendingPointAction::SplitLine, "M", a, b);
}
bool LineIntersectsLine3D(
    const glm::vec3& A,
    const glm::vec3& B,
    const glm::vec3& C,
    const glm::vec3& D,
    glm::vec3& hit,
    float eps)
{
    glm::vec3 u = B - A;
    glm::vec3 v = D - C;
    glm::vec3 w = A - C;

    float a = glm::dot(u, u);
    float b = glm::dot(u, v);
    float c = glm::dot(v, v);
    float d = glm::dot(u, w);
    float e = glm::dot(v, w);

    float denom = a * c - b * b;
    if (std::fabs(denom) < eps)
        return false;

    float s = (b * e - c * d) / denom;
    float t = (a * e - b * d) / denom;

    glm::vec3 p1 = A + s * u;
    glm::vec3 p2 = C + t * v;

    if (glm::length(p1 - p2) > eps)
        return false;

    hit = (p1 + p2) * 0.5f;
    return true;
}

bool Engine::Operations::ExtendUsingCutLine()
{
    if (Engine::Selecting::selectedOrder.size() != 4)
        return false;

    int ia = Engine::Selecting::selectedOrder[0];
    int ib = Engine::Selecting::selectedOrder[1];
    int ic = Engine::Selecting::selectedOrder[2];
    int id = Engine::Selecting::selectedOrder[3];

    if (ia == ib || ia == ic || ia == id || ib == ic || ib == id || ic == id)
        return false;

    glm::vec3 A = Engine::Buffers::positions[ia];
    glm::vec3 B = Engine::Buffers::positions[ib];
    glm::vec3 C = Engine::Buffers::positions[ic];
    glm::vec3 D = Engine::Buffers::positions[id];

    glm::vec3 hit;

    if (!LineIntersectsLine3D(A, B, C, D, hit, 1e-5f))
        return false;

    Engine::Buffers::AddPoint(hit.x, hit.y, hit.z, Engine::Buffers::pr,
                              Engine::Buffers::pg,
                              Engine::Buffers::pb,
                              Engine::Buffers::pa);

    Engine::Buffers::ConnectPointsLine(ia, ib);
    Engine::Buffers::ConnectPointsLine(ib, Engine::Buffers::positions.size() - 1);

    Engine::Buffers::Update();
    return true;
}

void Engine::Operations::PerpToPlane()
{
    int id = Engine::Selecting::selectedOrder[0];
    int ia = Engine::Selecting::selectedOrder[1];
    int ib = Engine::Selecting::selectedOrder[2];
    int ic = Engine::Selecting::selectedOrder[3];

    glm::vec3 D = Engine::Buffers::positions[id];
    glm::vec3 A = Engine::Buffers::positions[ia];
    glm::vec3 B = Engine::Buffers::positions[ib];
    glm::vec3 C = Engine::Buffers::positions[ic];

    glm::vec3 u = B - A;
    glm::vec3 v = C - A;

    glm::vec3 n = glm::cross(u, v);

    float nn = glm::dot(n, n);
    if (nn < 1e-8f)
        return;

    glm::vec3 w = D - A;

    float t = glm::dot(w, n) / nn;

    glm::vec3 perp = t * n;

    glm::vec3 H = D - perp;

    float dist = glm::length(perp);

    Engine::PointActions::RequestPointCreate(H, Engine::PointActions::PendingPointAction::PerpToPlane, "H", -1, -1, -1, -1, id);
}