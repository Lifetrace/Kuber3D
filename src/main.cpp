#include <iostream>
#include "Window.hpp"
#include "Events.hpp"
#include "Shader.hpp"
#include "Buffers.hpp"
#include "Camera.hpp"
#include "Ray.hpp"
#include "Grid.hpp"
#include "Cross.hpp"
#include "imgui.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ImGui_Style.hpp"

#include <unordered_map>
#include <algorithm>
#include <functional>

bool IsEditMode = false;
int shape = 8;
std::unordered_map<int, glm::vec4> selectedPointsByColor;
std::unordered_map<int, glm::vec3> selectedPointsByCoords;
std::vector<int> selectedOrder;
Engine::Camera cam;
glm::vec3 targetH = cam.target;

void SetEditMode();

void Select(int index);

void DeSelect(int index);

void DesAllSelected();

void DelAllSelected();

void ToggleSelect(int index);

int PickPointByPixels(const std::vector<glm::vec3> &positions, double mx, double my, const glm::mat4 &view, const glm::mat4 &proj, int w, int h, float radiusPx = 10.0f);

void CutLine(float p, float q);

int IndexByPos(const glm::vec3 &pos);

static void SortUnique(std::vector<int> &v);

bool RayPlane(const glm::vec3 &O, const glm::vec3 &D, const glm::vec3 &A, const glm::vec3 &n, glm::vec3 &hit);

bool ExtendUsingCutLine();

void CreateCube();
void CreatePyramid();
void CreateTetraheadron();
void CreateCircle(int N, float R, float cx, float cy, float cz);


int main()
{
    if (Engine::Window::Init(Engine::width, Engine::height, "Kuber 3D") != 0){
        return -1;
    }

    Engine::Events::initialize();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;

    Engine::Style::ApplyKuberOpaqueTheme();

    ImGui_ImplGlfw_InitForOpenGL(Engine::Window::window, true);
    ImGui_ImplOpenGL3_Init();

    ImFont *font = io.Fonts->AddFontFromFileTTF("../../assets/fonts/PFBeauSansPro-Reg.ttf", 16.0f);
    ImFont *font_bold = io.Fonts->AddFontFromFileTTF("../../assets/fonts/PFBeauSansPro-Bold.ttf", 16.0f);
    //io.Fonts->AddFontDefault();
    io.Fonts->Build();

    bool show_demo_window = false;

    Engine::Shader *shaderBase = Engine::load_shader("../../assets/shaders/basic.vert", "../../assets/shaders/basic.frag");
    if (!shaderBase)
        return -1;

    Engine::Shader* shaderLines = Engine::load_shader("../../assets/shaders/line.vert", "../../assets/shaders/line.frag", "../../assets/shaders/line.geom");
    if (!shaderLines)
        return -1;

    Engine::Buffers::Init();

    Engine::Grid::Init(1.0f, 100);

    glm::mat4 model(1.0f);

    Engine::Buffers::AddPoint(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    Engine::Buffers::AddPoint(1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
    Engine::Buffers::AddPoint(0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
    Engine::Buffers::AddPoint(0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

    Engine::Buffers::ConnectPointsLine(0, 1);
    Engine::Buffers::ConnectPointsLine(0, 2);
    Engine::Buffers::ConnectPointsLine(0, 3);

    while (!Engine::Window::isShouldClose(Engine::Window::GetWin()))
    {
        Engine::Events::PollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiIO &io = ImGui::GetIO();

        float smooth = 0.10f;
        cam.target = glm::mix(cam.target, targetH, smooth);

        int w, h;

        glfwGetFramebufferSize(Engine::Window::GetWin(), &w, &h);
        float aspect = (h == 0) ? 1.0f : (float)w / (float)h;

        glViewport(0, 0, w, h);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (Engine::Events::clicked(GLFW_MOUSE_BUTTON_LEFT))
        {
            cam.yaw += Engine::Events::deltaX * 0.007f;
            cam.pitch -= Engine::Events::deltaY * 0.007f;
            cam.pitch = glm::clamp(cam.pitch, -1.3f, 1.3f);
        }

        if (Engine::Events::jPressed(GLFW_KEY_P) && IsEditMode)
        {
            if (selectedOrder.size() != 3)
                continue;
            int i0 = selectedOrder[0];
            int i1 = selectedOrder[1];
            int i2 = selectedOrder[2];

            glm::vec3 A = Engine::Buffers::positions[i0];
            glm::vec3 B = Engine::Buffers::positions[i1];
            glm::vec3 C = Engine::Buffers::positions[i2];

            glm::vec3 n = glm::cross(B - A, C - A);
            float nn = glm::length(n);
            if (nn < 1e-6f)
                continue;
            n /= nn;

            double mx = Engine::Events::x;
            double my = Engine::Events::y;

            glm::mat4 view = cam.view();
            glm::mat4 proj = cam.proj((float)w / (float)h);

            glm::vec2 ndc = Engine::MouseToNDC(mx, my, w, h);
            Engine::Ray ray = Engine::MakeRayFromMouseNDC(ndc, proj, view);

            glm::vec3 hit;
            if (!RayPlane(ray.origin, ray.dir, A, n, hit))
                continue;

            Engine::Buffers::AddPoint(hit.x, hit.y, hit.z, 1.0f, 1.0f, 1.0f, 1.0f);
            Engine::Buffers::Update();
        }

        if (Engine::Events::jPressed(GLFW_KEY_K) && IsEditMode && selectedOrder.size() == 1)
        {
            int idx = selectedOrder[0];
            targetH = Engine::Buffers::positions[idx];
        }

        // DeletePoints
        if (Engine::Events::jPressed(GLFW_KEY_DELETE) && IsEditMode)
        {
            DelAllSelected();
        }

        // ConnectPoints
        if (Engine::Events::jPressed(GLFW_KEY_J) && IsEditMode && selectedOrder.size() == 2)
        {
            if (Engine::Events::Pressed(GLFW_KEY_LEFT_CONTROL))
                Engine::Buffers::DisConnectPointsLine(selectedOrder[0], selectedOrder[1]);
            else
                Engine::Buffers::ConnectPointsLine(selectedOrder[0], selectedOrder[1]);
        }

        // CutPoints
        if (Engine::Events::jPressed(GLFW_KEY_T) && IsEditMode && selectedOrder.size() == 2)
        {
            CutLine(1.0f, 2.0f);
        }

        // Tab
        if (Engine::Events::jPressed(GLFW_KEY_TAB))
        {
            SetEditMode();
        }

        if (Engine::Events::jPressed(GLFW_KEY_G) && IsEditMode)
        {
            if (!ExtendUsingCutLine())
            {
                continue;
            }
        
        }

        if (Engine::Events::jPressed(GLFW_KEY_N) && IsEditMode && Engine::Buffers::positions.size() == 0){
            CreateCircle(16, 1.0f, 0.0f, 0.0f, 0.0f);
        }

        if (Engine::Events::jPressed(GLFW_KEY_K) && IsEditMode &&
            Engine::Events::Pressed(GLFW_KEY_LEFT_CONTROL) &&
            Engine::Buffers::positions.size() == 0)
        {
            if (shape == 8){
                CreateCube();
            }

            if (shape == 5){
                CreatePyramid();
            }

            if (shape == 4){
                CreateTetraheadron();
            }
        }

        // Navigation
        if (Engine::Events::jclicked(GLFW_MOUSE_BUTTON_LEFT) && IsEditMode)
        {
            if (!Engine::Events::Pressed(GLFW_KEY_LEFT_SHIFT))
                DesAllSelected();

            double mx = Engine::Events::x;
            double my = Engine::Events::y;

            glm::vec2 ndc = Engine::MouseToNDC(mx, my, w, h);

            glm::mat4 view = cam.view();
            glm::mat4 proj = cam.proj((float)w / (float)h);

            Engine::Ray ray = Engine::MakeRayFromMouseNDC(ndc, proj, view);

            int idx = PickPointByPixels(Engine::Buffers::positions, mx, my, view, proj, w, h, 10.0f);
            if (idx != -1)
            {
                ToggleSelect(idx);
            }
        }

        // Shader Use and Set Matrixes
        shaderBase->use();
        shaderBase->SetMat4("uModel", model);
        shaderBase->SetMat4("uView", cam.view());
        shaderBase->SetMat4("uProj", cam.proj(aspect));

        // Color Selected Points
        for (const auto &[index, col] : selectedPointsByColor)
        {
            Engine::Buffers::ChangeColor(index, 0.5f, 0.5f, 1.0f, 1.0f);
        }

        // Defence
        if (Engine::Buffers::positions.size() < 2)
            Engine::Buffers::lineIndices.clear();

        // <--Grid Draw
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);

        Engine::Grid::Draw();

        glDepthMask(GL_TRUE);

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0f, 1.0f);

        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        Engine::Buffers::DrawFaces();
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        glDisable(GL_POLYGON_OFFSET_FILL);


        glDisable(GL_DEPTH_TEST);
        shaderBase->use();
        shaderBase->setInt("uIsPoints", 1);
        Engine::Buffers::DrawPoints();
        shaderBase->setInt("uIsPoints", 0);
        glEnable(GL_DEPTH_TEST);


        shaderLines->use();

        shaderLines->SetMat4("uModel", model);
        shaderLines->SetMat4("uView",  cam.view());
        shaderLines->SetMat4("uProj",  cam.proj(aspect));

        shaderLines->SetVec2("uViewport", glm::vec2((float)Engine::width, (float)Engine::height));
        shaderLines->SetFloat("uDashPx", 10.0f);
        shaderLines->SetFloat("uGapPx",  6.0f);

        glDepthMask(GL_FALSE);

        shaderLines->setInt("uDashed", 0);
        glDepthFunc(GL_LEQUAL);
        Engine::Buffers::DrawLines();

        shaderLines->setInt("uDashed", 1);
        glDepthFunc(GL_GEQUAL);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        Engine::Buffers::DrawLines();
        glDisable(GL_BLEND);

        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);


        shaderBase->use();

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_POLYGON_OFFSET_LINE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


        if (glfwGetWindowAttrib(Engine::Window::window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        if (show_demo_window)
        {
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        {
            static float f = 0.0f;
            static int counter = 0;
            ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize |
                                     ImGuiWindowFlags_NoMove |
                                     ImGuiWindowFlags_NoScrollbar |
                                     ImGuiWindowFlags_AlwaysAutoResize |
                                     ImGuiWindowFlags_NoCollapse |
                                     ImGuiWindowFlags_NoFocusOnAppearing |
                                     ImGuiWindowFlags_NoTitleBar;

            ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(330, -1), ImGuiCond_Always);

            ImGui::Begin("Kuber 3D", nullptr, flags);

            ImGui::PushFont(font_bold, 30.00f);
            ImGui::Text("Kuber 3D");
            ImGui::PopFont();

            ImGui::Separator();

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            if (ImGui::Button("Clean", ImVec2(-1, 0)))
            {
                if (IsEditMode){
                Engine::Buffers::DeleteAll();
                }
            }
            ImGui::SetItemTooltip("Tap to delete all points (need to be in edit mode)");
            ImGui::PopStyleColor();

            ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

            if (ImGui::BeginTabBar("Select", tab_bar_flags))
            {
                if (ImGui::BeginTabItem("Cube"))
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
                    ImGui::Text("Ctrl + K for Cube creation");
                    shape = 8;
                    ImGui::EndTabItem();
                    ImGui::PopStyleColor();
                }
                if (ImGui::BeginTabItem("Pyramid"))
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
                    ImGui::Text("Ctrl + K for Pyramid creation");
                    shape = 5;
                    ImGui::EndTabItem();
                    ImGui::PopStyleColor();
                }
                if (ImGui::BeginTabItem("Tetrahedron"))
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
                    ImGui::Text("Ctrl + K for Tetrahedron creation");
                    shape = 4;
                    ImGui::EndTabItem();
                    ImGui::PopStyleColor();
                }
                ImGui::EndTabBar();
            }
            ImGui::PopStyleColor();

            /*if (ImGui::Button("Filling", ImVec2(-1, 0)))
            {
                counter++;
            }

            ImGui::Text("Transparency");
            // ImGui::SliderFloat("##", &transparency, 0.0f, 1.0f, "0.%f");

            ImGui::Separator();

            if (ImGui::Button("Button"))
            {
                counter++;
            }
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Separator();
            static float custom_size = 16.0f;
            ImGui::SliderFloat("custom_size", &custom_size, 10.0f, 100.0f, "%.0f");
            ImGui::PushFont(NULL, custom_size);
            ImGui::Text("FontSize = %.2f (== %.2f * global_scale)", ImGui::GetFontSize(), custom_size);
            ImGui::PopFont();*/
            ImGui::Separator();

            ImGui::Text(" %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        Engine::Window::SwapBuffers(Engine::Window::GetWin());

        // Logs
        //
        //
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    Engine::Buffers::Destroy();
    Engine::Window::Terminate();

    return 0;
}

void SetEditMode()
{
    if (!IsEditMode)
    {
        IsEditMode = true;
        glClearColor(0.0f, 0.0f, 0.05f, 1.0f);
    }
    else
    {
        IsEditMode = false;
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    }
}

void Select(int index)
{
    if (index < 0 || index >= (int)Engine::Buffers::positions.size())
        return;
    if (selectedPointsByColor.contains(index))
        return;

    selectedPointsByColor.emplace(index, Engine::Buffers::colors[index]);
    selectedPointsByCoords.emplace(index, Engine::Buffers::positions[index]);
    selectedOrder.push_back(index);
}

void DeSelect(int index)
{
    if (index < 0 || index >= (int)Engine::Buffers::positions.size())
        return;

    auto it = selectedPointsByColor.find(index);
    if (it != selectedPointsByColor.end())
    {
        const glm::vec4 &c = it->second;
        Engine::Buffers::ChangeColor(index, c.r, c.g, c.b, c.a);
        selectedPointsByColor.erase(it);
    }

    selectedPointsByCoords.erase(index);

    selectedOrder.erase(
        std::remove(selectedOrder.begin(), selectedOrder.end(), index),
        selectedOrder.end());
}

void DesAllSelected()
{
    for (auto &[index, c] : selectedPointsByColor)
    {
        Engine::Buffers::ChangeColor(index, c.r, c.g, c.b, c.a);
    }

    selectedPointsByCoords.clear();
    selectedPointsByColor.clear();
    selectedOrder.clear();
}

void DelAllSelected()
{
    std::vector<int> selectedIdx = selectedOrder;
    SortUnique(selectedIdx);

    if (selectedIdx.empty())
        return;

    const int n = (int)Engine::Buffers::positions.size();

    std::vector<char> removed(n, 0);
    for (int idx : selectedIdx)
    {
        if (idx >= 0 && idx < n)
            removed[idx] = 1;
    }

    std::vector<int> mapOldToNew(n, -1);
    int newN = 0;
    for (int i = 0; i < n; i++)
    {
        if (!removed[i])
            mapOldToNew[i] = newN++;
    }

    std::vector<glm::vec3> newPos;
    newPos.reserve(newN);
    std::vector<glm::vec4> newCol;
    newCol.reserve(newN);

    for (int i = 0; i < n; i++)
    {
        if (!removed[i])
        {
            newPos.push_back(Engine::Buffers::positions[i]);
            newCol.push_back(Engine::Buffers::colors[i]);
        }
    }

    Engine::Buffers::positions = std::move(newPos);
    Engine::Buffers::colors = std::move(newCol);

    std::vector<GLuint> newLines;
    newLines.reserve(Engine::Buffers::lineIndices.size());

    for (size_t i = 0; i + 1 < Engine::Buffers::lineIndices.size(); i += 2)
    {
        int a = (int)Engine::Buffers::lineIndices[i];
        int b = (int)Engine::Buffers::lineIndices[i + 1];

        if (a < 0 || b < 0 || a >= n || b >= n)
            continue;
        if (removed[a] || removed[b])
            continue;

        newLines.push_back((GLuint)mapOldToNew[a]);
        newLines.push_back((GLuint)mapOldToNew[b]);
    }

    Engine::Buffers::lineIndices = std::move(newLines);

    if ((int)Engine::Buffers::connectedPoints.size() == n)
    {
        std::vector<std::vector<GLuint>> newAdj;
        newAdj.resize(newN);

        for (int i = 0; i < n; i++)
        {
            if (removed[i])
                continue;
            const int ni = mapOldToNew[i];

            for (GLuint nb : Engine::Buffers::connectedPoints[i])
            {
                int j = (int)nb;
                if (j < 0 || j >= n)
                    continue;
                if (removed[j])
                    continue;

                const GLuint nj = (GLuint)mapOldToNew[j];

                auto &row = newAdj[ni];
                if (std::find(row.begin(), row.end(), nj) == row.end())
                    row.push_back(nj);
            }
        }

        Engine::Buffers::connectedPoints = std::move(newAdj);
    }

    selectedOrder.clear();
    selectedPointsByColor.clear();
    selectedPointsByCoords.clear();

    Engine::Buffers::Update();
}

void ToggleSelect(int index)
{
    if (selectedPointsByColor.contains(index))
        DeSelect(index);
    else
        Select(index);
}

int PickPointByPixels(const std::vector<glm::vec3> &positions, double mx, double my, const glm::mat4 &view, const glm::mat4 &proj, int w, int h, float radiusPx)
{
    int best = -1;
    float bestD2 = radiusPx * radiusPx;

    glm::vec2 m((float)mx, (float)my);

    for (int i = 0; i < (int)positions.size(); i++)
    {
        glm::vec2 p;
        if (!Engine::WorldToScreen(positions[i], view, proj, w, h, p))
            continue;

        glm::vec2 d = p - m;
        float d2 = d.x * d.x + d.y * d.y;
        if (d2 <= bestD2)
        {
            bestD2 = d2;
            best = i;
        }
    }
    return best;
}

void CutLine(float p, float q)
{
    if (selectedOrder.size() < 2)
        return;
    if (q == 0)
        return;

    Engine::Buffers::DisConnectPointsLine(selectedOrder[0], selectedOrder[1]);

    float t = p / q;

    const int a = selectedOrder[0];
    const int b = selectedOrder[1];

    Engine::Buffers::DisConnectPointsLine(a, b);

    const glm::vec3 A = Engine::Buffers::positions[a];
    const glm::vec3 B = Engine::Buffers::positions[b];

    const glm::vec3 R = A + (B - A) * t;

    Engine::Buffers::AddPoint(R.x, R.y, R.z, 1.0f, 1.0f, 1.0f, 1.0f);

    const int newIndex = static_cast<int>(Engine::Buffers::positions.size()) - 1;
    Engine::Buffers::ConnectPointsLine(a, newIndex);
    Engine::Buffers::ConnectPointsLine(b, newIndex);
}

int IndexByPos(const glm::vec3 &pos)
{
    const float eps = 1e-5f;

    for (int i = 0; i < (int)Engine::Buffers::positions.size(); ++i)
    {
        const glm::vec3 &p = Engine::Buffers::positions[i];

        if (fabs(p.x - pos.x) < eps && fabs(p.y - pos.y) < eps && fabs(p.z - pos.z) < eps)
        {
            return i;
        }
    }
    return -1;
}

static void SortUnique(std::vector<int> &v)
{
    std::sort(v.begin(), v.end());
    v.erase(std::unique(v.begin(), v.end()), v.end());
}

bool RayPlane(
    const glm::vec3 &O, const glm::vec3 &D,
    const glm::vec3 &A, const glm::vec3 &n,
    glm::vec3 &hit)
{
    float denom = glm::dot(n, D);
    if (std::abs(denom) < 1e-6f)
        return false;

    float t = glm::dot(n, (A - O)) / denom;
    if (t <= 0.0f)
        return false;

    hit = O + t * D;
    return true;
}

bool ExtendUsingCutLine()
{

    if (selectedOrder.size() != 3)
        return false;

    int ia = selectedOrder[0];
    int ib = selectedOrder[1];
    int ic = selectedOrder[2];

    if (ia == ib || ia == ic || ib == ic)
        return false;

    glm::vec3 A = Engine::Buffers::positions[ia];
    glm::vec3 B = Engine::Buffers::positions[ib];
    glm::vec3 C = Engine::Buffers::positions[ic];

    glm::vec3 u3 = B - A;
    float ul = glm::length(u3);
    if (ul < 1e-6f)
        return false;
    u3 /= ul;

    glm::vec3 n = glm::cross(B - A, C - A);
    float nl = glm::length(n);
    if (nl < 1e-6f)
        return false;
    n /= nl;

    glm::vec3 v3 = glm::normalize(glm::cross(n, u3));

    auto To2 = [&](const glm::vec3 &X) -> glm::vec2
    {
        glm::vec3 d = X - A;
        return {glm::dot(d, u3), glm::dot(d, v3)};
    };

    glm::vec2 A2 = To2(A);
    glm::vec2 B2 = To2(B);
    glm::vec2 O = B2;
    glm::vec2 dir(1.0f, 0.0f);

    Engine::Buffers::ConnectPointsLine(ia, ib);

    int bestC = -1, bestD = -1;
    float bestT = 1e30f;
    float bestU = 0.0f;

    auto &li = Engine::Buffers::lineIndices;
    for (size_t i = 0; i + 1 < li.size(); i += 2)
    {
        int cIdx = (int)li[i];
        int dIdx = (int)li[i + 1];

        if ((cIdx == ia && dIdx == ib) || (cIdx == ib && dIdx == ia))
            continue;
        if (cIdx == ia || cIdx == ib || dIdx == ia || dIdx == ib)
            continue;

        glm::vec3 C3 = Engine::Buffers::positions[cIdx];
        glm::vec3 D3 = Engine::Buffers::positions[dIdx];

        float dc = std::abs(glm::dot(n, (C3 - A)));
        float dd = std::abs(glm::dot(n, (D3 - A)));
        if (dc > 1e-3f || dd > 1e-3f)
            continue;

        glm::vec2 C2 = To2(C3);
        glm::vec2 D2 = To2(D3);

        float t, u;
        if (!Engine::Cross::RaySeg2D(O, dir, C2, D2, t, u))
            continue;

        if (t < bestT)
        {
            bestT = t;
            bestU = u;
            bestC = cIdx;
            bestD = dIdx;
        }
    }

    if (bestC == -1)
        return false;

    auto saved = selectedOrder;
    selectedOrder = {bestC, bestD};

    float u = glm::clamp(bestU, 0.0f, 1.0f);
    float q = 1000.0f;
    float p = std::round(u * q);

    int oldCount = (int)Engine::Buffers::positions.size();
    CutLine(p, q);

    selectedOrder = saved;

    if ((int)Engine::Buffers::positions.size() == oldCount)
        return false;

    int newIndex = (int)Engine::Buffers::positions.size() - 1;

    Engine::Buffers::ConnectPointsLine(ib, newIndex);

    Engine::Buffers::Update();
    return true;
}

void CreateCube()
{
    Engine::Buffers::AddPoint(-1.0f, 0.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f); // 0
    Engine::Buffers::AddPoint(1.0f, 0.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f); // 1
    Engine::Buffers::AddPoint(-1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f); // 2
    Engine::Buffers::AddPoint(1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f); // 3

    Engine::Buffers::AddPoint(-1.0f, 2.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f); // 4
    Engine::Buffers::AddPoint(1.0f, 2.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f); // 5
    Engine::Buffers::AddPoint(-1.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f); // 6
    Engine::Buffers::AddPoint(1.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f); // 7

    // 0
    Engine::Buffers::ConnectPointsLine(0, 1);
    Engine::Buffers::ConnectPointsLine(0, 2);
    Engine::Buffers::ConnectPointsLine(0, 4);
    // 1
    Engine::Buffers::ConnectPointsLine(1, 5);
    Engine::Buffers::ConnectPointsLine(1, 3);
    // 3
    Engine::Buffers::ConnectPointsLine(3, 2);
    Engine::Buffers::ConnectPointsLine(3, 7);
    // 2
    Engine::Buffers::ConnectPointsLine(2, 6);
    // 4
    Engine::Buffers::ConnectPointsLine(4, 5);
    Engine::Buffers::ConnectPointsLine(4, 6);
    // 5
    Engine::Buffers::ConnectPointsLine(5, 7);
    // 7
    Engine::Buffers::ConnectPointsLine(7, 6);

    Engine::Buffers::AddQuad(0, 1, 3, 2);

    Engine::Buffers::AddQuad(4, 6, 7, 5);

    Engine::Buffers::AddQuad(2, 3, 7, 6);

    Engine::Buffers::AddQuad(0, 4, 5, 1);

    Engine::Buffers::AddQuad(0, 2, 6, 4);

    Engine::Buffers::AddQuad(1, 5, 7, 3);

    Engine::Buffers::Update();
}


void CreatePyramid()
{
    Engine::Buffers::AddPoint(-1.0f, 0.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f); // 0
    Engine::Buffers::AddPoint(1.0f, 0.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f); // 1
    Engine::Buffers::AddPoint(-1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f); // 2
    Engine::Buffers::AddPoint(1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f); // 3

    Engine::Buffers::AddPoint(0.0f, 2.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f); // 4


    // 0
    Engine::Buffers::ConnectPointsLine(0, 1);
    Engine::Buffers::ConnectPointsLine(0, 2);
    Engine::Buffers::ConnectPointsLine(0, 4);
    // 1
    Engine::Buffers::ConnectPointsLine(1, 4);
    Engine::Buffers::ConnectPointsLine(1, 3);
    // 3
    Engine::Buffers::ConnectPointsLine(3, 2);
    Engine::Buffers::ConnectPointsLine(3, 4);
    // 2
    Engine::Buffers::ConnectPointsLine(2, 4);

}


void CreateTetraheadron()
{
    Engine::Buffers::AddPoint(-1.0f, 0.0f, -0.8f, 1.0f, 1.0f, 1.0f, 1.0f); // 0
    Engine::Buffers::AddPoint(1.0f, 0.0f, -0.8f, 1.0f, 1.0f, 1.0f, 1.0f); // 1
    Engine::Buffers::AddPoint(0.0f, 0.0f, 1.3f, 1.0f, 1.0f, 1.0f, 1.0f); // 2

    Engine::Buffers::AddPoint(0.0f, 2.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f); // 3


    // 0
    Engine::Buffers::ConnectPointsLine(0, 1);
    Engine::Buffers::ConnectPointsLine(0, 2);
    Engine::Buffers::ConnectPointsLine(0, 3);
    // 1
    Engine::Buffers::ConnectPointsLine(1, 2);
    Engine::Buffers::ConnectPointsLine(1, 3);
    // 3
    Engine::Buffers::ConnectPointsLine(3, 2);
}

#include <cmath>

void CreateCircle(int N, float R, float cx, float cy, float cz)
{
    const float TWO_PI = 6.283185307179586f;

    for (int i = 0; i < N; ++i)
    {
        float t = TWO_PI * (float)i / (float)N;

        float x = cx + R * std::sin(t);
        float y = cy;
        float z = cz + R * std::cos(t);

        Engine::Buffers::AddPoint(x, y, z, 1.0f, 1.0f, 1.0f, 1.0f);
    }
}
