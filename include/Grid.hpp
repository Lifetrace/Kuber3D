#pragma once
#include "GLincs.hpp"
#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace Engine {

class Grid {
public:
    static void Init(float step = 1.0f, int halfSize = 50);
    static void Destroy();
    static void Draw();

    static inline float s_step = 1.0f;
    static inline int   s_half = 50;

private:
    static void Build();

    static inline GLuint vao = 0;
    static inline GLuint vboPos = 0;
    static inline GLuint vboCol = 0;

    static inline std::vector<glm::vec3> positions;
    static inline std::vector<glm::vec4> colors;
};

}
