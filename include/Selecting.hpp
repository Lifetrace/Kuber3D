#pragma once

#include <unordered_map>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <string>

namespace Engine
{
    class Selecting
    {
    public:
        static inline std::vector<std::string> pointNames;

        // Selections
        static inline std::unordered_map<int, glm::vec4> selectedPointsByColor;
        static inline std::unordered_map<int, glm::vec3> selectedPointsByCoords;
        static inline std::vector<int> selectedOrder;

        static void Select(int index);
        static void DeSelect(int index);
        static void DesAllSelected();
        static void DelAllSelected();
        static void ToggleSelect(int index);
    };
}