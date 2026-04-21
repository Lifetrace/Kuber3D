#pragma once

#include <vector>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace Engine
{
    class VectorMath
    {
    public:
        static void SortUnique(std::vector<int> &v);
        
        static int PickPointByPixels(const std::vector<glm::vec3> &positions, 
            double mx, 
            double my, 
            const glm::mat4 &view, 
            const glm::mat4 &proj, 
            int w, 
            int h, 
            float radiusPx = 10.0f);
            
        static bool RayIntersectsSegment3D(const glm::vec3 &A, 
            const glm::vec3 &B, 
            const glm::vec3 &C, 
            const glm::vec3 &D, 
            glm::vec3 &outHit, 
            float eps = 1e-5f);
    };
}