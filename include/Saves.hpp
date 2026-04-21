#pragma once

#include "Shader.hpp"

#include <string>

namespace Engine
{
    class Saves
    {
    public:
        static bool SaveSceneToFile(const std::string& path);
        static bool LoadSceneFromFile(const std::string& path);
        
        static std::string ShowSaveDialog();
        static std::string ShowOpenDialog();
    };
}