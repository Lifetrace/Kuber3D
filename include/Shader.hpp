#pragma once
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

namespace Engine{
    class Shader {
    public:
        unsigned int ID = 0;

        Shader() = default;
        Shader(const std::string& vertexPath, const std::string& fragmentPath);

        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;

        Shader(Shader&& other) noexcept;
        Shader& operator=(Shader&& other) noexcept;

        ~Shader();

        void use() const;

        void setBool (const std::string& name, bool value) const;
        void setInt  (const std::string& name, int value) const;
        void setFloat(const std::string& name, float value) const;

        void setVec2(const std::string& name, const glm::vec2& v) const;
        void setVec3(const std::string& name, const glm::vec3& v) const;
        void setVec4(const std::string& name, const glm::vec4& v) const;

        void setMat3(const std::string& name, const glm::mat3& m) const;
        void setMat4(const std::string& name, const glm::mat4& m) const;

    private:
        mutable std::unordered_map<std::string, int> _uniformCache;

        static std::string readFile(const std::string& path);
        static unsigned int compileStage(unsigned int type, const std::string& source, const std::string& debugName);
        static unsigned int linkProgram(unsigned int vs, unsigned int fs);

        int getLocation(const std::string& name) const;
    };
}