#include "ImGui_Style.hpp"
#include "imgui.h"
#include "Buffers.hpp"


void Engine::Style::ApplyKuberLightTheme()
{
    ImGuiStyle &style = ImGui::GetStyle();
    // General
    style.FontSizeBase = 20.00f;
    style.Colors[ImGuiCol_Text] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);

    // Colors
    style.Colors[ImGuiCol_WindowBg] = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);
    style.Colors[ImGuiCol_Tab] = ImVec4(0.0f, 0.2f, 0.6f, 1.00f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.0f, 0.2f, 0.6f, 1.00f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.8f, 0.8f, 0.8f, 1.00f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.9f, 0.9f, 0.9f, 1.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.17f, 0.21f, 1.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);

    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.0f, 0.2f, 0.6f, 1.00f);

    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.0f, 0.2f, 0.6f, 1.00f);
    // style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 0.1f, 0.1f, 1.00f);

    style.Colors[ImGuiCol_Button] = ImVec4(0.0f, 0.2f, 0.6f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.0f, 0.3f, 0.7f, 1.00f);

    // Main
    style.WindowPadding = ImVec2(5.0f, 5.0f);
    style.FramePadding = ImVec2(1.0f, 5.0f);
    style.ItemSpacing = ImVec2(4.0f, 6.0f);
    style.ItemInnerSpacing = ImVec2(4.0f, 0.0f);
    style.TouchExtraPadding = ImVec2(0.0f, 0.0f);
    style.IndentSpacing = 20.0f;
    style.GrabMinSize = 20.0f;
    style.WindowBorderSize = 0.0f;

    // Rounding
    style.WindowRounding = 5.0f;
    style.ChildRounding = 5.0f;
    style.FrameRounding = 5.0f;
    style.PopupRounding = 5.0f;
    style.GrabRounding = 5.0f;

    Engine::Buffers::pr = 0.0f;
    Engine::Buffers::pg = 0.2f;
    Engine::Buffers::pb = 0.6f;
    Engine::Buffers::pa = 1.0f;

    for (int i; i < Engine::Buffers::positions.size(); i++){
        Engine::Buffers::ChangeColor(i, Engine::Buffers::pr, Engine::Buffers::pg, Engine::Buffers::pb, Engine::Buffers::pa);
    }
}

void Engine::Style::ApplyKuberDarkTheme()
{
    ImGuiStyle &style = ImGui::GetStyle();
    // General
    style.FontSizeBase = 20.00f;
    style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

    // Colors
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.00f);
    style.Colors[ImGuiCol_Tab] = ImVec4(0.0f, 0.2f, 0.6f, 1.00f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.0f, 0.2f, 0.6f, 1.00f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.00f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.3f, 0.3f, 0.3f, 1.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.17f, 0.21f, 1.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);

    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.0f, 0.2f, 0.6f, 1.00f);

    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.0f, 0.2f, 0.6f, 1.00f);
    // style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 0.1f, 0.1f, 1.00f);

    style.Colors[ImGuiCol_Button] = ImVec4(0.0f, 0.2f, 0.6f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.0f, 0.3f, 0.7f, 1.00f);

    // Main
    style.WindowPadding = ImVec2(5.0f, 5.0f);
    style.FramePadding = ImVec2(1.0f, 5.0f);
    style.ItemSpacing = ImVec2(4.0f, 6.0f);
    style.ItemInnerSpacing = ImVec2(4.0f, 0.0f);
    style.TouchExtraPadding = ImVec2(0.0f, 0.0f);
    style.IndentSpacing = 20.0f;
    style.GrabMinSize = 20.0f;
    style.WindowBorderSize = 0.0f;

    // Rounding
    style.WindowRounding = 5.0f;
    style.ChildRounding = 5.0f;
    style.FrameRounding = 5.0f;
    style.PopupRounding = 5.0f;
    style.GrabRounding = 5.0f;

    Engine::Buffers::pr = 1.0f;
    Engine::Buffers::pg = 1.0f;
    Engine::Buffers::pb = 1.0f;
    Engine::Buffers::pa = 1.0f;

    for (int i; i < Engine::Buffers::positions.size(); i++){
        Engine::Buffers::ChangeColor(i, Engine::Buffers::pr, Engine::Buffers::pg, Engine::Buffers::pb, Engine::Buffers::pa);
    }
}
