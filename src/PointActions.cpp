#include "PointActions.hpp"

#include "Buffers.hpp"

#include "imgui.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ImGui_Style.hpp"

#include "Ray.hpp"

#include "Window.hpp"

Engine::PointActions::PendingLineUI Engine::PointActions::gPendingLine{};
Engine::PointActions::PendingPointUI Engine::PointActions::gPendingPoint{};
Engine::PointActions::PendingRenameUI Engine::PointActions::gRename{};

void Engine::PointActions::DrawLineLabels(
    const glm::mat4 &view,
    const glm::mat4 &proj,
    int w, int h)
{
    ImDrawList *dl = ImGui::GetBackgroundDrawList();
    if (!dl)
        return;

    ImU32 col = (Engine::Style::Theme == 0) ? IM_COL32(255, 255, 255, 255) : IM_COL32(0, 71, 171, 255);
    const ImVec2 off(6.0f, 6.0f);

    // пары индексов: [a0,b0,a1,b1,...]
    auto &L = Engine::Buffers::lineIndices;

    for (int seg = 0; seg + 1 < (int)L.size(); seg += 2)
    {
        int a = (int)L[seg];
        int b = (int)L[seg + 1];
        if (a < 0 || b < 0 || a >= (int)Engine::Buffers::positions.size() || b >= (int)Engine::Buffers::positions.size())
            continue;

        glm::vec3 mid = (Engine::Buffers::positions[a] + Engine::Buffers::positions[b]) * 0.5f;

        glm::vec2 p;
        if (!Engine::WorldToScreen(mid, view, proj, w, h, p))
            continue;

        uint64_t key = LineKey(a, b);
        auto it = lineNamesByKey.find(key);

        if (it == lineNamesByKey.end() || it->second.empty())
            continue;

        dl->AddText(ImVec2(p.x + off.x, p.y + off.y), col, it->second.c_str());
    }
}

void Engine::PointActions::RequestLineLabel(int a, int b, const char *defaultName)
{
    gPendingLine.active = true;
    gPendingLine.a = a;
    gPendingLine.b = b;
    std::snprintf(gPendingLine.name, sizeof(gPendingLine.name), "%s", defaultName);
}

void Engine::PointActions::DrawLineNamePopup()
{
    if (!gPendingLine.active)
        return;

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar;
    ImGui::SetNextWindowPos(ImVec2(Engine::width / 2 - 38, Engine::height / 2 - 60), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(-1, 0), ImGuiCond_Always);

    ImGui::Begin("LineName", nullptr, flags);

    ImGui::Text("Name Line?");
    ImGui::InputText("##LineName", gPendingLine.name, IM_ARRAYSIZE(gPendingLine.name));

    ImGui::Separator();

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
    if (ImGui::Button("Set", ImVec2(-1, 0)))
    {
        uint64_t key = LineKey(gPendingLine.a, gPendingLine.b);
        lineNamesByKey[key] = std::string(gPendingLine.name);

        gPendingLine.active = false;
        gPendingLine.a = gPendingLine.b = -1;
    }
    if (ImGui::Button("Cancel", ImVec2(-1, 0)))
    {
        gPendingLine.active = false;
        gPendingLine.a = gPendingLine.b = -1;
    }
    ImGui::PopStyleColor();

    ImGui::End();
}

void Engine::PointActions::RequestPointCreate(
    const glm::vec3 &pos,
    PendingPointAction action,
    const char *defaultName,
    int a, int b, int ia, int ib, int id)
{
    gPendingPoint.active = true;
    gPendingPoint.pos = pos;
    gPendingPoint.action = action;
    gPendingPoint.a = a;
    gPendingPoint.b = b;
    gPendingPoint.ia = ia;
    gPendingPoint.ib = ib;
    gPendingPoint.id = id;

    std::snprintf(gPendingPoint.name, sizeof(gPendingPoint.name), "%s", defaultName);
}

void Engine::PointActions::RequestRenamePoint(int idx, const std::string &current)
{
    gRename.active = true;
    gRename.mode = RenameMode::Point;
    gRename.pointIndex = idx;
    gRename.a = gRename.b = -1;
    std::snprintf(gRename.name, sizeof(gRename.name), "%s", current.c_str());
}

void Engine::PointActions::RequestRenameLine(int a, int b, const std::string &current)
{
    gRename.active = true;
    gRename.mode = RenameMode::Line;
    gRename.pointIndex = -1;
    gRename.a = a;
    gRename.b = b;
    std::snprintf(gRename.name, sizeof(gRename.name), "%s", current.c_str());
}

void Engine::PointActions::DrawRenamePopup()
{
    if (!gRename.active)
        return;

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar;

    ImGui::SetNextWindowPos(ImVec2(Engine::width / 2 - 60, Engine::height / 2 - 60), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(-1, 0), ImGuiCond_Always);

    ImGui::Begin("Rename", nullptr, flags);

    if (gRename.mode == RenameMode::Point)
        ImGui::Text("Rename point:");
    else if (gRename.mode == RenameMode::Line)
        ImGui::Text("Rename line:");
    else
        ImGui::Text("Rename:");

    ImGui::InputText("##rename", gRename.name, IM_ARRAYSIZE(gRename.name));

    ImGui::Separator();

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));

    if (ImGui::Button("Set", ImVec2(-1, 0)))
    {
        std::string s = gRename.name;

        if (gRename.mode == RenameMode::Point)
        {
            if (gRename.pointIndex >= 0)
            {
                pointNames.resize(Engine::Buffers::positions.size());
                pointNames[gRename.pointIndex] = s;
            }
        }
        else if (gRename.mode == RenameMode::Line)
        {
            uint64_t key = LineKey(gRename.a, gRename.b);

            if (s.empty())
                lineNamesByKey.erase(key);
            else
                lineNamesByKey[key] = s;
        }

        gRename.active = false;
        gRename.mode = RenameMode::None;
        gRename.pointIndex = -1;
        gRename.a = gRename.b = -1;
        gRename.name[0] = '\0';
    }

    if (ImGui::Button("Cancel", ImVec2(-1, 0)))
    {
        gRename.active = false;
        gRename.mode = RenameMode::None;
        gRename.pointIndex = -1;
        gRename.a = gRename.b = -1;
        gRename.name[0] = '\0';
    }

    ImGui::PopStyleColor();

    ImGui::End();
}

void Engine::PointActions::DrawPointNamePopup()
{
    if (!gPendingPoint.active)
        return;

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar;

    ImGui::SetNextWindowPos(ImVec2(Engine::width / 2 - 38, Engine::height / 2 - 60), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(-1, 0), ImGuiCond_Always);

    ImGui::Begin("PointName", nullptr, flags);

    ImGui::Text("Point name:");
    ImGui::InputText("##PointName", gPendingPoint.name, IM_ARRAYSIZE(gPendingPoint.name));

    ImGui::Separator();

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    if (ImGui::Button("Create", ImVec2(-1, 0)))
    {
        // 1) Add point
        Engine::Buffers::AddPoint(gPendingPoint.pos.x, gPendingPoint.pos.y, gPendingPoint.pos.z, Engine::Buffers::pr, Engine::Buffers::pg, Engine::Buffers::pb, Engine::Buffers::pa);

        int newIndex = (int)Engine::Buffers::positions.size() - 1;

        // 2) store name (keep vector aligned with positions)
        pointNames.resize(Engine::Buffers::positions.size());
        pointNames[newIndex] = std::string(gPendingPoint.name);

        // 3) post actions
        if (gPendingPoint.action == PendingPointAction::SplitLine)
        {
            if (gPendingPoint.a >= 0 && gPendingPoint.b >= 0)
            {
                Engine::Buffers::ConnectPointsLine(gPendingPoint.a, newIndex);
                Engine::Buffers::ConnectPointsLine(gPendingPoint.b, newIndex);
            }
        }
        else if (gPendingPoint.action == PendingPointAction::ExtendLine)
        {
            if (gPendingPoint.ia >= 0 && gPendingPoint.ib >= 0)
            {
                Engine::Buffers::ConnectPointsLine(gPendingPoint.ia, gPendingPoint.ib);
                Engine::Buffers::ConnectPointsLine(gPendingPoint.ib, newIndex);
            }
        }
        else if (gPendingPoint.action == PendingPointAction::PerpToPlane)
        {
            if (gPendingPoint.id >= 0)
            {
                Engine::Buffers::ConnectPointsLine(gPendingPoint.id, newIndex);
            }
        }

        Engine::Buffers::Update();

        // close
        gPendingPoint.active = false;
        gPendingPoint.action = PendingPointAction::None;
    }

    if (ImGui::Button("Cancel", ImVec2(-1, 0)))
    {
        gPendingPoint.active = false;
        gPendingPoint.action = PendingPointAction::None;
    }
    ImGui::PopStyleColor();

    ImGui::End();
}

void Engine::PointActions::DrawPointLabels(
    const std::vector<glm::vec3> &positions,
    const glm::mat4 &view,
    const glm::mat4 &proj,
    int w, int h)
{
    ImDrawList *dl = ImGui::GetBackgroundDrawList();
    if (!dl)
        return;

    ImU32 col = (Engine::Style::Theme == 0) ? IM_COL32(255, 255, 255, 255) : IM_COL32(0, 71, 171, 255);
    const ImVec2 off(6.0f, -6.0f);

    for (int i = 0; i < (int)positions.size(); ++i)
    {
        glm::vec2 p;
        if (!Engine::WorldToScreen(positions[i], view, proj, w, h, p))
            continue;

        std::string label;
        if (i >= 0 && i < (int)pointNames.size() && !pointNames[i].empty())
            label = pointNames[i];
        else
            label = GetPointLabel(i);
        dl->AddText(ImVec2(p.x + off.x, p.y + off.y), col, label.c_str());
    }
}

void Engine::PointActions::ResetSceneNames()
{
    pointNames.clear();
    lineNamesByKey.clear();

    gPendingLine = PendingLineUI{};
    gPendingPoint = PendingPointUI{};
    gRename = PendingRenameUI{};
}