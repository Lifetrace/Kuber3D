#include "Grid.hpp"
#include <cmath>

static inline glm::vec4 MinorLineColor() { return {0.0f, 0.0f, 0.0f, 0.6f}; }
static inline glm::vec4 AxisXColor()     { return {0.0f, 0.2f, 0.6f, 0.8f}; }
static inline glm::vec4 AxisZColor()     { return {0.0f, 0.2f, 0.6f, 0.8f}; }

static inline void PushLine(
    std::vector<glm::vec3>& pos,
    std::vector<glm::vec4>& col,
    const glm::vec3& a,
    const glm::vec3& b,
    const glm::vec4& c
){
    pos.push_back(a); col.push_back(c);
    pos.push_back(b); col.push_back(c);
}

void Engine::Grid::Init(float step, int halfSize) {
    s_step = (step <= 0.0f) ? 1.0f : step;
    s_half = (halfSize < 1) ? 1 : halfSize;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vboPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glGenBuffers(1, &vboCol);
    glBindBuffer(GL_ARRAY_BUFFER, vboCol);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindVertexArray(0);

    Build();
}

void Engine::Grid::Build() {
    positions.clear();
    colors.clear();

    const float half = (float)s_half;
    const float step = s_step;

    for (float z = -half; z <= half + 0.0001f; z += step) {
        glm::vec4 c = (std::abs(z) < 0.0001f) ? AxisXColor() : MinorLineColor();
        PushLine(positions, colors, {-half, 0.0f, z}, {+half, 0.0f, z}, c);
    }

    for (float x = -half; x <= half + 0.0001f; x += step) {
        glm::vec4 c = (std::abs(x) < 0.0001f) ? AxisZColor() : MinorLineColor();
        PushLine(positions, colors, {x, 0.0f, -half}, {x, 0.0f, +half}, c);
    }

    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glBufferData(GL_ARRAY_BUFFER,
        (GLsizeiptr)(positions.size() * sizeof(glm::vec3)),
        positions.data(),
        GL_DYNAMIC_DRAW
    );

    glBindBuffer(GL_ARRAY_BUFFER, vboCol);
    glBufferData(GL_ARRAY_BUFFER,
        (GLsizeiptr)(colors.size() * sizeof(glm::vec4)),
        colors.data(),
        GL_DYNAMIC_DRAW
    );
}

void Engine::Grid::Draw() {
    if (!vao) return;

    glBindVertexArray(vao);
    glDrawArrays(GL_LINES, 0, (GLsizei)positions.size());
    glBindVertexArray(0);
}

void Engine::Grid::Destroy() {
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vboPos) glDeleteBuffers(1, &vboPos);
    if (vboCol) glDeleteBuffers(1, &vboCol);

    vao = vboPos = vboCol = 0;
    positions.clear();
    colors.clear();
}
