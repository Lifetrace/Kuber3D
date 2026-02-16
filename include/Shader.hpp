#pragma once

#include "GLincs.hpp"
#include <string>
#include <glm/mat4x4.hpp>
#include <unordered_map>
#include <glm/gtc/type_ptr.hpp>

typedef unsigned int uint;

namespace Engine{
    class Shader{
    public:
        uint id;

        Shader(uint id);
        ~Shader();

        void setInt(const std::string& name, int value);
        void SetMat4(const std::string& name, const glm::mat4& m);
        void SetFloat(const std::string& name, float value);
        void SetVec2(const std::string& name, glm::vec2 v);

        void use();

    private:
        GLint getUniformLocation(const std::string name);
        std::unordered_map<std::string, GLint> m_cache;
    };

    Shader* load_shader(std::string vPath, std::string fPath);
    Shader* load_shader(std::string vPath, std::string fPath, std::string gPath);
    std::string ReadTextFile(std::string Path);
}