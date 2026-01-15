#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>


namespace Engine{

class Shader; // твой класс шейдера

class TetrahedronWire {
    public:
        TetrahedronWire() = default;

        void Init(float size = 1.0f);

        void Draw(const Engine::Shader& shader, const glm::mat4& mvp) const;

        void Destroy();

    private:
        GLuint _vao = 0;
        GLuint _vbo = 0;
        GLuint _ebo = 0;

        GLsizei _indexCount = 0;
    };
}