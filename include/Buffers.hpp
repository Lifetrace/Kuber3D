#pragma once
#include "GLincs.hpp"
#include <algorithm>
#include <vector>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <unordered_map>

typedef unsigned int uint;

namespace Engine{
    class Buffers{
        static bool HasNeighbor(const std::vector<GLuint>& v, GLuint x){
            return std::find(v.begin(), v.end(), x) != v.end();
        }
        static void RemoveNeighbor(std::vector<GLuint>& v, GLuint x){
            v.erase(std::remove(v.begin(), v.end(), x), v.end());
        }
    public:
        // base dots color
        static float inline pr = 0.0f;
        static float inline pg = 0.2f;
        static float inline pb = 0.6f;
        static float inline pa = 1.0f;
        
        static void Update();
        static inline GLuint vao, vboPos, vboCol, eboFaces;

        static inline std::vector<glm::vec3> positions;
        static inline std::vector<glm::vec4> colors;
        static inline std::vector<GLuint> lineIndices;
        static inline std::vector<std::vector<GLuint>> connectedPoints;
        static inline std::vector<unsigned int> faceIndices;

        static inline GLuint eboLines = 0;

        static void Init();

        static void AddPoint(float x, float y, float z, float r, float g, float b, float a);
        static void DeletePoint(int index);
        static void DeleteAll();
        static void ConnectPointsLine(int first, int second);
        static void DisConnectPointsLine(int first, int second);
        
        static void AddFace(unsigned int a, unsigned int b, unsigned int c);
        static void AddQuad(unsigned int a, unsigned int b, unsigned int c, unsigned int d);

        static void ChangeColor(int index, float r, float g, float b, float a);

        static void Destroy();

        static void DrawLines();
        static void DrawPoints();
        static void DrawFaces();
    };
}