#pragma once

#include <string>
#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace Engine
{
    class Saves
    {
    public:
        enum class SceneKind
        {
            User = 0,
            Lesson = 1
        };

        struct LineNameEntry
        {
            int a = -1;
            int b = -1;
            std::string name;
        };

        struct SceneSnapshot
        {
            std::vector<glm::vec3> positions;
            std::vector<glm::vec4> colors;
            std::vector<unsigned int> lineIndices;
            std::vector<unsigned int> faceIndices;
            std::vector<std::string> pointNames;
            std::vector<LineNameEntry> lineNames;
        };

        struct SceneDocument
        {
            SceneKind kind = SceneKind::User;
            SceneSnapshot base;
            std::vector<SceneSnapshot> steps;
            int currentStep = 0;
            std::string title;
            bool readOnly = false;
        };

        static SceneDocument gCurrentDocument;
        static bool gHasDocument;

        static SceneSnapshot CaptureCurrentScene();
        static void ApplySnapshot(const SceneSnapshot &snap);

        static void NewUserScene();
        static void NewLessonScene();

        static bool SaveDocumentToFile(const std::string &path);
        static bool LoadDocumentFromFile(const std::string &path);

        static std::string ShowSaveDialog();
        static std::string ShowOpenDialog();

        static bool IsLessonMode()
        {
            return gHasDocument && gCurrentDocument.kind == SceneKind::Lesson;
        }

        static bool IsReadOnly()
        {
            return gHasDocument && gCurrentDocument.readOnly;
        }

        static bool IsLessonAuthorMode()
        {
            return gHasDocument && gCurrentDocument.kind == SceneKind::Lesson && !gCurrentDocument.readOnly;
        }

        static bool IsLessonViewerMode()
        {
            return gHasDocument &&  gCurrentDocument.kind == SceneKind::Lesson && gCurrentDocument.readOnly;
        }

        static void SetReadOnly(bool value)
        {
            if (!gHasDocument)
                return;

            gCurrentDocument.readOnly = value;
        }
        static void AddLessonStepFromCurrent();
        static void ReplaceCurrentLessonStepFromCurrent();
        static void DeleteCurrentLessonStep();

        static void PrevLessonStep();
        static void NextLessonStep();
    };
}