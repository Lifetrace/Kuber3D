#pragma once
#include "GLincs.hpp"
#include <algorithm>
#include <vector>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <unordered_map>


namespace Engine{
    class Buffers{
        static bool HasNeighbor(const std::vector<GLuint>& v, GLuint x){
            return std::find(v.begin(), v.end(), x) != v.end();
        }
        static void RemoveNeighbor(std::vector<GLuint>& v, GLuint x){
            v.erase(std::remove(v.begin(), v.end(), x), v.end());
        }
    public:
        static void Update();
        static inline GLuint vao, vboPos, vboCol;

        static inline std::vector<glm::vec3> positions;
        static inline std::vector<glm::vec4> colors;
        static inline std::vector<GLuint> lineIndices;
        static inline std::vector<std::vector<GLuint>> connectedPoints;

        static inline GLuint eboLines = 0;

        static void Init();

        static void AddPoint(float x, float y, float z, float r, float g, float b, float a);
        static void DeletePoint(int index);
        static void DeleteAll();
        static void ConnectPointsLine(int first, int second);
        static void DisConnectPointsLine(int first, int second);

        static void ChangeColor(int index, float r, float g, float b, float a);

        static void Destroy();

        static void DrawLines();
        static void DrawPoints();
    };
}