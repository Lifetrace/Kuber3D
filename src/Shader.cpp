#include "Shader.hpp"

#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>


Engine::Shader::Shader(uint id) : id(id){

}

GLint Engine::Shader::getUniformLocation(const std::string name)
    {
    auto it = m_cache.find(name);
    if (it != m_cache.end())
        return it->second;
    GLint loc = glGetUniformLocation(id, name.c_str());
    m_cache[name] = loc;
    return loc;
}

Engine::Shader::~Shader(){
    glDeleteProgram(id);
}
void Engine::Shader::setInt(const std::string& name, int value){

        glUseProgram(id);

        GLint loc = getUniformLocation(name);
        if (loc == -1) return; 
        glUniform1i(loc, value);
}

void Engine::Shader::SetMat4(const std::string& name, const glm::mat4& m){
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &m[0][0]);
}

void Engine::Shader::use(){
    glUseProgram(id);
}
std::string Engine::ReadTextFile(std::string Path){
    std::ifstream file(Path, std::ios::in);
    if(!file.is_open())
    throw std::runtime_error("Engine::ReadTextFile: cannot open file: " + Path);

    std::ostringstream ss;
    ss << file.rdbuf();

    if (!file.good() && !file.eof())
        throw std::runtime_error("Engine::ReadTextFile: read error: " + Path);

    return ss.str();
}

Engine::Shader* Engine::load_shader(std::string vPath, std::string fPath)
{
    std::string vSrc = Engine::ReadTextFile(vPath);
    std::string fSrc = Engine::ReadTextFile(fPath);

    const char* vShaderCode = vSrc.c_str();
    const char* fShaderCode = fSrc.c_str();

    GLuint v = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(v, 1, &vShaderCode, nullptr);
    glCompileShader(v);

    GLuint f = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(f, 1, &fShaderCode, nullptr);
    glCompileShader(f);

    GLuint pr = glCreateProgram();
    glAttachShader(pr, v);
    glAttachShader(pr, f);
    glLinkProgram(pr);

    glDeleteShader(v);
    glDeleteShader(f);

    return new Engine::Shader(pr);
}

