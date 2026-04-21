#include "Saves.hpp"

#include "Buffers.hpp"
#include "PointActions.hpp"
#include "Selecting.hpp"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#pragma comment(lib, "Comdlg32.lib")
#endif

Engine::Saves::SceneDocument Engine::Saves::gCurrentDocument{};
bool Engine::Saves::gHasDocument = false;

namespace
{
    using Engine::Saves;

    static void ResetRuntimeSceneState()
    {
        Engine::Selecting::selectedPointsByColor.clear();
        Engine::Selecting::selectedPointsByCoords.clear();
        Engine::Selecting::selectedOrder.clear();

        Engine::PointActions::pointNames.clear();
        Engine::PointActions::lineNamesByKey.clear();

        Engine::PointActions::gPendingLine = Engine::PointActions::PendingLineUI{};
        Engine::PointActions::gPendingPoint = Engine::PointActions::PendingPointUI{};
        Engine::PointActions::gRename = Engine::PointActions::PendingRenameUI{};
    }

    static Saves::LineNameEntry MakeLineNameEntryFromKey(uint64_t key, const std::string &name)
    {
        Saves::LineNameEntry e{};
        e.a = (int)(key >> 32);
        e.b = (int)(key & 0xFFFFFFFFu);
        e.name = name;
        return e;
    }

    static std::string KindToString(Saves::SceneKind kind)
    {
        return (kind == Saves::SceneKind::Lesson) ? "LESSON" : "USER";
    }

    static Saves::SceneKind StringToKind(const std::string &s)
    {
        if (s == "LESSON")
            return Saves::SceneKind::Lesson;
        return Saves::SceneKind::User;
    }

    static void WriteSnapshot(std::ostream &out, const Saves::SceneSnapshot &snap, const std::string &tag)
    {
        out << "SNAPSHOT_BEGIN " << tag << "\n";

        out << "POINTS " << snap.positions.size() << "\n";
        for (size_t i = 0; i < snap.positions.size(); ++i)
        {
            const glm::vec3 &p = snap.positions[i];
            glm::vec4 c(0.0f, 0.2f, 0.6f, 1.0f);
            if (i < snap.colors.size())
                c = snap.colors[i];

            std::string pointName;
            if (i < snap.pointNames.size())
                pointName = snap.pointNames[i];

            out << std::setprecision(9)
                << p.x << ' ' << p.y << ' ' << p.z << ' '
                << c.r << ' ' << c.g << ' ' << c.b << ' ' << c.a << ' '
                << std::quoted(pointName) << "\n";
        }

        out << "LINES " << (snap.lineIndices.size() / 2) << "\n";
        for (size_t i = 0; i + 1 < snap.lineIndices.size(); i += 2)
        {
            out << snap.lineIndices[i] << ' ' << snap.lineIndices[i + 1] << "\n";
        }

        out << "FACES " << (snap.faceIndices.size() / 3) << "\n";
        for (size_t i = 0; i + 2 < snap.faceIndices.size(); i += 3)
        {
            out << snap.faceIndices[i] << ' ' << snap.faceIndices[i + 1] << ' ' << snap.faceIndices[i + 2] << "\n";
        }

        out << "LINENAMES " << snap.lineNames.size() << "\n";
        for (const auto &e : snap.lineNames)
        {
            out << e.a << ' ' << e.b << ' ' << std::quoted(e.name) << "\n";
        }

        out << "SNAPSHOT_END " << tag << "\n";
    }

    static bool ReadSnapshot(std::istream &in, Saves::SceneSnapshot &snap, std::string &outBeginTag)
    {
        snap = Saves::SceneSnapshot{};

        std::string word;
        if (!(in >> word))
            return false;

        if (word != "SNAPSHOT_BEGIN")
            return false;

        if (!(in >> outBeginTag))
            return false;

        // POINTS
        size_t pointsCount = 0;
        if (!(in >> word >> pointsCount) || word != "POINTS")
            return false;

        snap.positions.reserve(pointsCount);
        snap.colors.reserve(pointsCount);
        snap.pointNames.reserve(pointsCount);

        for (size_t i = 0; i < pointsCount; ++i)
        {
            float x, y, z, r, g, b, a;
            std::string pointName;

            if (!(in >> x >> y >> z >> r >> g >> b >> a >> std::quoted(pointName)))
                return false;

            snap.positions.emplace_back(x, y, z);
            snap.colors.emplace_back(r, g, b, a);
            snap.pointNames.push_back(pointName);
        }

        // LINES
        size_t linesCount = 0;
        if (!(in >> word >> linesCount) || word != "LINES")
            return false;

        snap.lineIndices.reserve(linesCount * 2);
        for (size_t i = 0; i < linesCount; ++i)
        {
            unsigned int a, b;
            if (!(in >> a >> b))
                return false;

            snap.lineIndices.push_back(a);
            snap.lineIndices.push_back(b);
        }

        // FACES
        size_t facesCount = 0;
        if (!(in >> word >> facesCount) || word != "FACES")
            return false;

        snap.faceIndices.reserve(facesCount * 3);
        for (size_t i = 0; i < facesCount; ++i)
        {
            unsigned int a, b, c;
            if (!(in >> a >> b >> c))
                return false;

            snap.faceIndices.push_back(a);
            snap.faceIndices.push_back(b);
            snap.faceIndices.push_back(c);
        }

        // LINENAMES
        size_t lineNamesCount = 0;
        if (!(in >> word >> lineNamesCount) || word != "LINENAMES")
            return false;

        snap.lineNames.reserve(lineNamesCount);
        for (size_t i = 0; i < lineNamesCount; ++i)
        {
            Saves::LineNameEntry e;
            if (!(in >> e.a >> e.b >> std::quoted(e.name)))
                return false;

            snap.lineNames.push_back(e);
        }

        // END
        std::string endTag;
        if (!(in >> word >> endTag))
            return false;

        if (word != "SNAPSHOT_END")
            return false;

        if (endTag != outBeginTag)
            return false;

        return true;
    }
}

Engine::Saves::SceneSnapshot Engine::Saves::CaptureCurrentScene()
{
    SceneSnapshot snap{};

    snap.positions = Engine::Buffers::positions;
    snap.colors = Engine::Buffers::colors;
    snap.lineIndices.assign(Engine::Buffers::lineIndices.begin(), Engine::Buffers::lineIndices.end());
    snap.faceIndices = Engine::Buffers::faceIndices;
    snap.pointNames = Engine::PointActions::pointNames;

    snap.lineNames.reserve(Engine::PointActions::lineNamesByKey.size());
    for (const auto &[key, name] : Engine::PointActions::lineNamesByKey)
    {
        snap.lineNames.push_back(MakeLineNameEntryFromKey(key, name));
    }

    std::sort(snap.lineNames.begin(), snap.lineNames.end(),
              [](const LineNameEntry &l, const LineNameEntry &r)
              {
                  if (l.a != r.a)
                      return l.a < r.a;
                  if (l.b != r.b)
                      return l.b < r.b;
                  return l.name < r.name;
              });

    return snap;
}

void Engine::Saves::ApplySnapshot(const SceneSnapshot &snap)
{
    ResetRuntimeSceneState();
    Engine::Buffers::DeleteAll();

    for (size_t i = 0; i < snap.positions.size(); ++i)
    {
        const glm::vec3 &p = snap.positions[i];
        glm::vec4 c(Engine::Buffers::pr, Engine::Buffers::pg, Engine::Buffers::pb, Engine::Buffers::pa);

        if (i < snap.colors.size())
            c = snap.colors[i];

        Engine::Buffers::AddPoint(p.x, p.y, p.z, c.r, c.g, c.b, c.a);
    }

    Engine::PointActions::pointNames = snap.pointNames;
    if (Engine::PointActions::pointNames.size() < Engine::Buffers::positions.size())
        Engine::PointActions::pointNames.resize(Engine::Buffers::positions.size());

    for (size_t i = 0; i + 1 < snap.lineIndices.size(); i += 2)
    {
        int a = (int)snap.lineIndices[i];
        int b = (int)snap.lineIndices[i + 1];
        Engine::Buffers::ConnectPointsLine(a, b);
    }

    for (size_t i = 0; i + 2 < snap.faceIndices.size(); i += 3)
    {
        Engine::Buffers::AddFace(
            snap.faceIndices[i],
            snap.faceIndices[i + 1],
            snap.faceIndices[i + 2]);
    }

    Engine::PointActions::lineNamesByKey.clear();
    for (const auto &e : snap.lineNames)
    {
        Engine::PointActions::lineNamesByKey[Engine::PointActions::LineKey(e.a, e.b)] = e.name;
    }

    Engine::Buffers::Update();
}

void Engine::Saves::NewUserScene()
{
    gCurrentDocument = SceneDocument{};
    gCurrentDocument.kind = SceneKind::User;
    gCurrentDocument.base = CaptureCurrentScene();
    gCurrentDocument.steps.clear();
    gCurrentDocument.currentStep = 0;
    gCurrentDocument.title = "User Scene";
    gHasDocument = true;
}

void Engine::Saves::NewLessonScene()
{
    gCurrentDocument = SceneDocument{};
    gCurrentDocument.kind = SceneKind::Lesson;
    gCurrentDocument.readOnly = false;
    gCurrentDocument.base = CaptureCurrentScene();
    gCurrentDocument.steps.clear();
    gCurrentDocument.steps.push_back(gCurrentDocument.base);
    gCurrentDocument.currentStep = 0;
    gCurrentDocument.title = "Lesson Scene";
    gHasDocument = true;
}

bool Engine::Saves::SaveDocumentToFile(const std::string &path)
{
    if (path.empty())
        return false;

    if (!gHasDocument)
    {
        NewUserScene();
    }
    if (gCurrentDocument.kind == SceneKind::User)
    {
        gCurrentDocument.base = CaptureCurrentScene();
    }

    std::ofstream out(path);
    if (!out.is_open())
        return false;

    out << "KUBER3D_DOC_V2\n";
    out << "KIND " << KindToString(gCurrentDocument.kind) << "\n";
    out << "TITLE " << std::quoted(gCurrentDocument.title) << "\n";
    out << "CURRENT_STEP " << gCurrentDocument.currentStep << "\n";

    if (gCurrentDocument.kind == SceneKind::User)
    {
        WriteSnapshot(out, gCurrentDocument.base, "BASE");
    }
    else
    {
        out << "STEPS " << gCurrentDocument.steps.size() << "\n";
        for (size_t i = 0; i < gCurrentDocument.steps.size(); ++i)
        {
            WriteSnapshot(out, gCurrentDocument.steps[i], "STEP_" + std::to_string(i));
        }
    }

    return true;
}

bool Engine::Saves::LoadDocumentFromFile(const std::string &path)
{

    if (path.empty())
        return false;

    std::ifstream in(path);
    if (!in.is_open())
        return false;

    std::string magic;
    if (!(in >> magic))
        return false;

    if (magic != "KUBER3D_DOC_V2")
        return false;

    std::string word;
    std::string kindStr;
    if (!(in >> word >> kindStr) || word != "KIND")
        return false;

    SceneDocument doc{};
    doc.kind = StringToKind(kindStr);
    
    if (doc.kind == SceneKind::Lesson)
    {
        doc.readOnly = true;
    }
    else
    {
        doc.readOnly = false;
    }

    if (!(in >> word) || word != "TITLE")
        return false;
    if (!(in >> std::quoted(doc.title)))
        return false;

    if (!(in >> word >> doc.currentStep) || word != "CURRENT_STEP")
        return false;

    if (doc.kind == SceneKind::User)
    {
        std::string tag;
        if (!ReadSnapshot(in, doc.base, tag))
            return false;

        ApplySnapshot(doc.base);
        doc.steps.clear();
        doc.currentStep = 0;
    }
    else
    {
        size_t stepsCount = 0;
        if (!(in >> word >> stepsCount) || word != "STEPS")
            return false;

        doc.steps.clear();
        doc.steps.reserve(stepsCount);

        for (size_t i = 0; i < stepsCount; ++i)
        {
            SceneSnapshot step;
            std::string tag;
            if (!ReadSnapshot(in, step, tag))
                return false;

            doc.steps.push_back(std::move(step));
        }

        if (doc.steps.empty())
        {
            doc.base = SceneSnapshot{};
            doc.currentStep = 0;
            ApplySnapshot(doc.base);
        }
        else
        {
            if (doc.currentStep < 0)
                doc.currentStep = 0;
            if (doc.currentStep >= (int)doc.steps.size())
                doc.currentStep = (int)doc.steps.size() - 1;

            doc.base = doc.steps.front();
            ApplySnapshot(doc.steps[doc.currentStep]);
        }
    }

    gCurrentDocument = std::move(doc);
    gHasDocument = true;

    return true;
}

std::string Engine::Saves::ShowSaveDialog()
{
#ifdef _WIN32
    char fileName[MAX_PATH] = "scene.kub";

    OPENFILENAMEA ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFilter = "Kuber scene (*.kub)\0*.kub\0All files (*.*)\0*.*\0";
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
    ofn.lpstrFilter = "Kuber scene (*.kub)\0*.kub\0All files (*.*)\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileNameA(&ofn))
        return std::string(fileName);
#endif
    return "";
}

void Engine::Saves::AddLessonStepFromCurrent()
{
    if (!gHasDocument || gCurrentDocument.kind != SceneKind::Lesson)
        return;

    SceneSnapshot snap = CaptureCurrentScene();
    gCurrentDocument.steps.push_back(std::move(snap));
    gCurrentDocument.currentStep = (int)gCurrentDocument.steps.size() - 1;
    gCurrentDocument.base = gCurrentDocument.steps.front();
}

void Engine::Saves::ReplaceCurrentLessonStepFromCurrent()
{
    if (!gHasDocument || gCurrentDocument.kind != SceneKind::Lesson)
        return;
    if (gCurrentDocument.steps.empty())
        return;
    if (gCurrentDocument.currentStep < 0 || gCurrentDocument.currentStep >= (int)gCurrentDocument.steps.size())
        return;

    gCurrentDocument.steps[gCurrentDocument.currentStep] = CaptureCurrentScene();
    gCurrentDocument.base = gCurrentDocument.steps.front();
}

void Engine::Saves::DeleteCurrentLessonStep()
{
    if (!gHasDocument || gCurrentDocument.kind != SceneKind::Lesson)
        return;
    if (gCurrentDocument.steps.empty())
        return;
    if (gCurrentDocument.currentStep < 0 || gCurrentDocument.currentStep >= (int)gCurrentDocument.steps.size())
        return;

    gCurrentDocument.steps.erase(gCurrentDocument.steps.begin() + gCurrentDocument.currentStep);

    if (gCurrentDocument.steps.empty())
    {
        gCurrentDocument.base = SceneSnapshot{};
        gCurrentDocument.currentStep = 0;
        ApplySnapshot(gCurrentDocument.base);
        return;
    }

    if (gCurrentDocument.currentStep >= (int)gCurrentDocument.steps.size())
        gCurrentDocument.currentStep = (int)gCurrentDocument.steps.size() - 1;

    gCurrentDocument.base = gCurrentDocument.steps.front();
    ApplySnapshot(gCurrentDocument.steps[gCurrentDocument.currentStep]);
}

void Engine::Saves::PrevLessonStep()
{
    if (!gHasDocument || gCurrentDocument.kind != SceneKind::Lesson)
        return;
    if (gCurrentDocument.steps.empty())
        return;

    if (gCurrentDocument.currentStep > 0)
    {
        --gCurrentDocument.currentStep;
        ApplySnapshot(gCurrentDocument.steps[gCurrentDocument.currentStep]);
    }
}

void Engine::Saves::NextLessonStep()
{
    if (!gHasDocument || gCurrentDocument.kind != SceneKind::Lesson)
        return;
    if (gCurrentDocument.steps.empty())
        return;

    if (gCurrentDocument.currentStep + 1 < (int)gCurrentDocument.steps.size())
    {
        ++gCurrentDocument.currentStep;
        ApplySnapshot(gCurrentDocument.steps[gCurrentDocument.currentStep]);
    }
}