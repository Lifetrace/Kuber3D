#include "Saves.hpp"

#include "Buffers.hpp"
#include "PointActions.hpp"
#include "Selecting.hpp"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <unordered_map>

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#endif

static std::string EscapeName(const std::string& s)
{
    std::ostringstream out;
    out << std::quoted(s);
    return out.str();
}

static bool ReadQuoted(std::istringstream& iss, std::string& out)
{
    iss >> std::quoted(out);
    return !iss.fail();
}

std::string Engine::Saves::ShowSaveDialog()
{
#ifdef _WIN32
    char fileName[MAX_PATH] = "scene.kub";

    OPENFILENAMEA ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFilter = "Kuber Scene (*.kub)\0*.kub\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrDefExt = "kub";
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

    if (GetSaveFileNameA(&ofn))
        return std::string(fileName);
#endif
    return "";
}

std::string Engine::Saves::ShowOpenDialog()
{
#ifdef _WIN32
    char fileName[MAX_PATH] = "";

    OPENFILENAMEA ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFilter = "Kuber Scene (*.kub)\0*.kub\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileNameA(&ofn))
        return std::string(fileName);
#endif
    return "";
}

bool Engine::Saves::SaveSceneToFile(const std::string& path)
{
    std::ofstream out(path);
    if (!out.is_open())
        return false;

    out << "KUBER3D_SCENE_V1\n";

    out << "POINTS " << Engine::Buffers::positions.size() << "\n";
    for (size_t i = 0; i < Engine::Buffers::positions.size(); ++i)
    {
        const auto& p = Engine::Buffers::positions[i];
        const auto& c = Engine::Buffers::colors[i];

        std::string name;
        if (i < Engine::PointActions::pointNames.size())
            name = Engine::PointActions::pointNames[i];

        out << p.x << ' ' << p.y << ' ' << p.z << ' '
            << c.r << ' ' << c.g << ' ' << c.b << ' ' << c.a << ' '
            << EscapeName(name) << "\n";
    }

    out << "LINES " << Engine::Buffers::lineIndices.size() / 2 << "\n";
    for (size_t i = 0; i + 1 < Engine::Buffers::lineIndices.size(); i += 2)
    {
        int a = (int)Engine::Buffers::lineIndices[i];
        int b = (int)Engine::Buffers::lineIndices[i + 1];
        out << a << ' ' << b << "\n";
    }

    out << "FACES " << Engine::Buffers::faceIndices.size() / 3 << "\n";
    for (size_t i = 0; i + 2 < Engine::Buffers::faceIndices.size(); i += 3)
    {
        out << Engine::Buffers::faceIndices[i] << ' '
            << Engine::Buffers::faceIndices[i + 1] << ' '
            << Engine::Buffers::faceIndices[i + 2] << "\n";
    }

    out << "LINENAMES " << Engine::PointActions::lineNamesByKey.size() << "\n";
    for (const auto& [key, name] : Engine::PointActions::lineNamesByKey)
    {
        int a = (int)(key >> 32);
        int b = (int)(key & 0xFFFFFFFF);
        out << a << ' ' << b << ' ' << EscapeName(name) << "\n";
    }

    return true;
}

bool Engine::Saves::LoadSceneFromFile(const std::string& path)
{
    std::ifstream in(path);
    if (!in.is_open())
        return false;

    std::string header;
    std::getline(in, header);
    if (header != "KUBER3D_SCENE_V1")
        return false;

    Engine::Selecting::ResetSelection();
    Engine::PointActions::ResetSceneNames();
    Engine::Buffers::DeleteAll();

    std::string tag;
    size_t count = 0;

    // POINTS
    in >> tag >> count;
    if (tag != "POINTS")
        return false;

    Engine::PointActions::pointNames.clear();

    for (size_t i = 0; i < count; ++i)
    {
        float x, y, z, r, g, b, a;
        in >> x >> y >> z >> r >> g >> b >> a;

        std::string name;
        in >> std::quoted(name);

        Engine::Buffers::AddPoint(x, y, z, r, g, b, a);
        Engine::PointActions::pointNames.push_back(name);
    }

    // LINES
    in >> tag >> count;
    if (tag != "LINES")
        return false;

    for (size_t i = 0; i < count; ++i)
    {
        int a, b;
        in >> a >> b;
        Engine::Buffers::ConnectPointsLine(a, b);
    }

    // FACES
    in >> tag >> count;
    if (tag != "FACES")
        return false;

    for (size_t i = 0; i < count; ++i)
    {
        unsigned int a, b, c;
        in >> a >> b >> c;
        Engine::Buffers::AddFace(a, b, c);
    }

    // LINENAMES
    in >> tag >> count;
    if (tag != "LINENAMES")
        return false;

    for (size_t i = 0; i < count; ++i)
    {
        int a, b;
        std::string name;
        in >> a >> b >> std::quoted(name);

        Engine::PointActions::lineNamesByKey[Engine::PointActions::LineKey(a, b)] = name;
    }

    Engine::Buffers::Update();
    return true;
}