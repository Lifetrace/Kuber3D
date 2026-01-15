#include "Shader.hpp"

#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <utility>

Engine::Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    const std::string vsrc = readFile(vertexPath);
    const std::string fsrc = readFile(fragmentPath);

    unsigned int vs = compileStage(GL_VERTEX_SHADER, vsrc, vertexPath);
    unsigned int fs = compileStage(GL_FRAGMENT_SHADER, fsrc, fragmentPath);

    ID = linkProgram(vs, fs);

    glDeleteShader(vs);
    glDeleteShader(fs);
}

Engine::Shader::Shader(Shader&& other) noexcept {
    ID = other.ID;
    _uniformCache = std::move(other._uniformCache);
    other.ID = 0;
}

Engine::Shader& Engine::Shader::operator=(Shader&& other) noexcept {
    if (this == &other) return *this;

    if (ID) glDeleteProgram(ID);

    ID = other.ID;
    _uniformCache = std::move(other._uniformCache);
    other.ID = 0;
    return *this;
}

Engine::Shader::~Shader() {
    if (ID) glDeleteProgram(ID);
}

void Engine::Shader::use() const {
    glUseProgram(ID);
}

std::string Engine::Shader::readFile(const std::string& path) {
    std::ifstream file(path, std::ios::in);
    if (!file.is_open()) {
        throw std::runtime_error("Shader file not found: " + path);
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

unsigned int Engine::Shader::compileStage(unsigned int type, const std::string& source, const std::string& debugName) {
    unsigned int shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        int len = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, '\0');
        glGetShaderInfoLog(shader, len, nullptr, log.data());

        std::cerr << "Shader compile error (" << debugName << ")\n" << log << "\n";
        glDeleteShader(shader);
        throw std::runtime_error("Shader compilation failed: " + debugName);
    }
    return shader;
}

unsigned int Engine::Shader::linkProgram(unsigned int vs, unsigned int fs) {
    unsigned int program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    int ok = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        int len = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, '\0');
        glGetProgramInfoLog(program, len, nullptr, log.data());

        std::cerr << "Program link error:\n" << log << "\n";
        glDeleteProgram(program);
        throw std::runtime_error("Program linking failed");
    }
    return program;
}

int Engine::Shader::getLocation(const std::string& name) const {
    auto it = _uniformCache.find(name);
    if (it != _uniformCache.end()) return it->second;

    int loc = glGetUniformLocation(ID, name.c_str());
    _uniformCache[name] = loc;
    return loc;
}

void Engine::Shader::setBool(const std::string& name, bool value) const {
    glUniform1i(getLocation(name), (int)value);
}
void Engine::Shader::setInt(const std::string& name, int value) const {
    glUniform1i(getLocation(name), value);
}
void Engine::Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(getLocation(name), value);
}

void Engine::Shader::setVec2(const std::string& name, const glm::vec2& v) const {
    glUniform2f(getLocation(name), v.x, v.y);
}
void Engine::Shader::setVec3(const std::string& name, const glm::vec3& v) const {
    glUniform3f(getLocation(name), v.x, v.y, v.z);
}
void Engine::Shader::setVec4(const std::string& name, const glm::vec4& v) const {
    glUniform4f(getLocation(name), v.x, v.y, v.z, v.w);
}

void Engine::Shader::setMat3(const std::string& name, const glm::mat3& m) const {
    glUniformMatrix3fv(getLocation(name), 1, GL_FALSE, &m[0][0]);
}
void Engine::Shader::setMat4(const std::string& name, const glm::mat4& m) const {
    glUniformMatrix4fv(getLocation(name), 1, GL_FALSE, &m[0][0]);
}
