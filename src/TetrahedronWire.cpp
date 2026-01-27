#include "TetrahedronWire.hpp"
#include "Shader.hpp"        
#include <glm/gtc/type_ptr.hpp>

void Engine::TetrahedronWire::Init(float size) {
    const float s = size;

    const float vertices[] = {
         0.0f,  s,   0.0f,      
        -s,   -s,   s,           
         s,   -s,   s,             
         0.0f, -s, -s              
    };

    // 6 рёбер => 12 индексов под GL_LINES
    const unsigned int indices[] = {
        0,1,
        0,2,
        0,3,
        1,2,
        1,3,
        2,3
    };
    _indexCount = (GLsizei)(sizeof(indices) / sizeof(indices[0]));

    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);
    glGenBuffers(1, &_ebo);

    glBindVertexArray(_vao);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // layout(location=0) -> vec3 position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);
}

void Engine::TetrahedronWire::Draw(const Engine::Shader& shader, const glm::mat4& mvp) const{
    shader.use();
    shader.setMat4("uMVP", mvp);

    glBindVertexArray(_vao);

    glDisable(GL_CULL_FACE);
   
    glDrawElements(GL_LINES, _indexCount, GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);
}

void Engine::TetrahedronWire::Destroy() {
    if (_ebo) glDeleteBuffers(1, &_ebo);
    if (_vbo) glDeleteBuffers(1, &_vbo);
    if (_vao) glDeleteVertexArrays(1, &_vao);

    _ebo = _vbo = _vao = 0;
    _indexCount = 0;
}
