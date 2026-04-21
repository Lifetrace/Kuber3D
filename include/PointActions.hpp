#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <string>

namespace Engine
{
    class PointActions
    {
    public:
        enum class PendingPointAction
        {
            None = 0,
            JustAdd,    // add point only
            SplitLine,  // add point + connect to (a,b)
            ExtendLine, // add point + connect ia->ib and ib->new
            PerpToPlane // add point + connect id->new
        };

        struct PendingPointUI
        {
            bool active = false;
            glm::vec3 pos{0, 0, 0};

            PendingPointAction action = PendingPointAction::None;

            // action data
            int a = -1, b = -1;   // SplitLine: connect a-new and b-new
            int ia = -1, ib = -1; // ExtendLine: connect ia-ib and ib-new
            int id = -1;          // PerpToPlane: connect id-new

            char name[32] = "P"; // input buffer
        };

        struct PendingLineUI
        {
            bool active = false;
            int a = -1, b = -1;
            char name[16] = "1";
        };

        static PendingLineUI gPendingLine;
        static PendingPointUI gPendingPoint;

        // Drawing

        static void DrawLineLabels(
            const glm::mat4 &view,
            const glm::mat4 &proj,
            int w, int h);

        static void RequestLineLabel(int a, int b, const char *defaultName);
        static void DrawLineNamePopup();

        static void RequestPointCreate(
            const glm::vec3 &pos,
            PendingPointAction action,
            const char *defaultName = "P",
            int a = -1, int b = -1, int ia = -1, int ib = -1, int id = -1);

        // Renaming
        enum class RenameMode
        {
            None,
            Point,
            Line
        };

        struct PendingRenameUI
        {
            bool active = false;
            RenameMode mode = RenameMode::None;

            int pointIndex = -1;
            int a = -1, b = -1;

            char name[32] = "";
        };

        static PendingRenameUI gRename;

        static inline std::unordered_map<uint64_t, std::string> lineNamesByKey;

        static void RequestRenamePoint(int idx, const std::string &current);
        static void RequestRenameLine(int a, int b, const std::string &current);

        static inline std::vector<std::string> pointNames;

        static void DrawRenamePopup();

        static uint64_t LineKey(int a, int b)
        {
            int lo = std::min(a, b);
            int hi = std::max(a, b);
            return ((uint64_t)(uint32_t)lo << 32) | (uint64_t)(uint32_t)hi;
        }
        static void DrawPointNamePopup();
        static void DrawPointLabels(
            const std::vector<glm::vec3> &positions,
            const glm::mat4 &view,
            const glm::mat4 &proj,
            int w, int h);
        static void ResetSceneNames();
    };

    inline std::string GetPointLabel(int i)
    {
        static const char *k8[] = {"A", "B", "C", "D", "A1", "B1", "C1", "D1"};
        if (i >= 0 && i < 8)
            return k8[i];

        std::string res = (i == 8) ? "P" : "P" + std::to_string(i - 8);
        return res;
    }
}