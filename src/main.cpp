/*#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#include "Image_loader.h"

// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTextureFromMemory(const void *data, size_t data_size, GLuint *out_texture, int *out_width, int *out_height)
{
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char *image_data = stbi_load_from_memory((const unsigned char *)data, (int)data_size, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload pixels into texture
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}

// Open and read a file, then forward to LoadTextureFromMemory()
bool LoadTextureFromFile(const char *file_name, GLuint *out_texture, int *out_width, int *out_height)
{
    FILE *f = fopen(file_name, "rb");
    if (f == NULL)
        return false;
    fseek(f, 0, SEEK_END);
    size_t file_size = (size_t)ftell(f);
    if (file_size == -1)
        return false;
    fseek(f, 0, SEEK_SET);
    void *file_data = IM_ALLOC(file_size);
    fread(file_data, 1, file_size, f);
    fclose(f);
    bool ret = LoadTextureFromMemory(file_data, file_size, out_texture, out_width, out_height);
    IM_FREE(file_data);
    return ret;
}
*/

#include <iostream>
#include <cmath>
#include "Window.hpp"
#include "Events.hpp"
#include "Shader.hpp"
#include "Buffers.hpp"
#include "Camera.hpp"
#include "Ray.hpp"
#include "Grid.hpp"
#include "Cross.hpp"
#include "Angle.hpp"
#include "imgui.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ImGui_Style.hpp"

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

#include <unordered_map>
#include <algorithm>
#include <functional>

// Extra parameters
int Theme = 0;
bool IsEditMode = false;
bool Split = false;
int shape = 8;

// Angles
std::vector<Engine::Angle> angles;

// Selections
std::unordered_map<int, glm::vec4> selectedPointsByColor;
std::unordered_map<int, glm::vec3> selectedPointsByCoords;
std::vector<int> selectedOrder;

// Camera
Engine::Camera cam;
glm::vec3 targetH = cam.target;

// Set Edit Mode
void SetEditMode();

// User selecting
void Select(int index);
void DeSelect(int index);
void DesAllSelected();
void DelAllSelected();
void ToggleSelect(int index);

// Operations
void SetSplit();
void CutLine(float p, float q);
bool ExtendUsingCutLine();
void PerpToPlane();

// Vector Maths
int PickPointByPixels(const std::vector<glm::vec3> &positions, double mx, double my, const glm::mat4 &view, const glm::mat4 &proj, int w, int h, float radiusPx = 10.0f);
static void SortUnique(std::vector<int> &v);

// Figures
void CreateCube();
void CreateCuboid();
void CreatePyramid();
void CreateTetraheadron();
void CreateCircle(int N, float R, float cx, float cy, float cz); // in dev

// Letters
std::string GetPointLabel(int i);
static void DrawPointLabels(
    const std::vector<glm::vec3> &positions,
    const glm::mat4 &view,
    const glm::mat4 &proj,
    int w, int h);

// Future
int IndexByPos(const glm::vec3 &pos);

// Standart points colors
float pr = Engine::Buffers::pr;
float pg = Engine::Buffers::pg;
float pb = Engine::Buffers::pb;
float pa = Engine::Buffers::pa;

// Point names (same order as Engine::Buffers::positions)
std::vector<std::string> pointNames;

// Pending point creation popup
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

static PendingPointUI gPendingPoint;

void RequestPointCreate(
    const glm::vec3 &pos,
    PendingPointAction action,
    const char *defaultName = "P",
    int a = -1, int b = -1, int ia = -1, int ib = -1, int id = -1);

void DrawPointNamePopup();

// Line labels by key [ min(a,b), max(a,b) ] -> 1, 2, ...
static std::unordered_map<uint64_t, std::string> lineNamesByKey;

static uint64_t LineKey(int a, int b)
{
    int lo = std::min(a, b);
    int hi = std::max(a, b);
    return ((uint64_t)(uint32_t)lo << 32) | (uint64_t)(uint32_t)hi;
}
struct PendingLineUI
{
    bool active = false;
    int a = -1, b = -1;
    char name[16] = "1";
};

static PendingLineUI gPendingLine;

void DrawLineLabels(
    const glm::mat4 &view,
    const glm::mat4 &proj,
    int w, int h);
void RequestLineLabel(int a, int b, const char *defaultName);
void DrawLineNamePopup();

bool HasLineBetween(int a, int b);
enum class RenameMode
{
    None,
    Point,
    Line
};

// Rename Labels
struct PendingRenameUI
{
    bool active = false;
    RenameMode mode = RenameMode::None;

    int pointIndex = -1;
    int a = -1, b = -1;

    char name[32] = "";
};

static PendingRenameUI gRename;
void RequestRenamePoint(int idx, const std::string &current);
void RequestRenameLine(int a, int b, const std::string &current);
void DrawRenamePopup();

int main()
{
    if (Engine::Window::Init(Engine::width, Engine::height, "Kuber 3D") != 0)
    {
        return -1;
    }

    /*int my_image_width = 0;
    int my_image_height = 0;
    GLuint my_image_texture = 0;
    bool ret = LoadTextureFromFile("../../assets/images/kuber_3d-logo.svg", &my_image_texture, &my_image_width, &my_image_height);
    IM_ASSERT(ret);
    */

    Engine::Events::initialize();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;

    ImGui_ImplGlfw_InitForOpenGL(Engine::Window::window, true);
    ImGui_ImplOpenGL3_Init();

    ImFont *font = io.Fonts->AddFontFromFileTTF("../../assets/fonts/PFBeauSansPro-Reg.ttf", 16.0f);
    ImFont *font_bold = io.Fonts->AddFontFromFileTTF("../../assets/fonts/PFBeauSansPro-Bold.ttf", 16.0f);
    // io.Fonts->AddFontDefault();
    io.Fonts->Build();

    bool show_demo_window = false;

    Engine::Shader *shaderBase = Engine::load_shader("../../assets/shaders/basic.vert", "../../assets/shaders/basic.frag");
    if (!shaderBase)
        return -1;

    Engine::Shader *shaderLines = Engine::load_shader("../../assets/shaders/line.vert", "../../assets/shaders/line.frag", "../../assets/shaders/line.geom");
    if (!shaderLines)
        return -1;

    Engine::Buffers::Init();

    Engine::Grid::Init(1.0f, 100);

    glm::mat4 model(1.0f);

    Engine::Buffers::AddPoint(0.0f, 0.0f, 0.0f, pr, pg, pb, pa);
    Engine::Buffers::AddPoint(1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
    Engine::Buffers::AddPoint(0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
    Engine::Buffers::AddPoint(0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

    Engine::Buffers::ConnectPointsLine(0, 1);
    Engine::Buffers::ConnectPointsLine(0, 2);
    Engine::Buffers::ConnectPointsLine(0, 3);

    while (!Engine::Window::isShouldClose(Engine::Window::GetWin()))
    {
        Engine::Events::PollEvents();

        if (Engine::Events::jPressed(GLFW_KEY_F11))
        {
            Engine::Window::ToggleFullscreen();
        }

        if (glfwGetWindowAttrib(Engine::Window::window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiIO &io = ImGui::GetIO();

        const bool uiCapturesKeyboard = io.WantTextInput || io.WantCaptureKeyboard;

        const bool blockHotkeys = uiCapturesKeyboard || gPendingPoint.active || Split;

        if (Theme == 1)
        {
            Engine::Style::ApplyKuberLightTheme();
        }
        else
        {
            Engine::Style::ApplyKuberDarkTheme();
        }

        float smooth = 0.10f;
        cam.target = glm::mix(cam.target, targetH, smooth);

        int w, h;

        glfwGetFramebufferSize(Engine::Window::GetWin(), &w, &h);
        float aspect = (h == 0) ? 1.0f : (float)w / (float)h;

        glViewport(0, 0, w, h);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (!blockHotkeys)
        {
            if (Engine::Events::clicked(GLFW_MOUSE_BUTTON_LEFT))
            {
                cam.yaw += Engine::Events::deltaX * 0.007f;
                cam.pitch -= Engine::Events::deltaY * 0.007f;
                cam.pitch = glm::clamp(cam.pitch, -1.3f, 1.3f);
            }

            if (Engine::Events::jPressed(GLFW_KEY_R) && IsEditMode && selectedOrder.size() == 4)
            {
                PerpToPlane();
            }

            if (Engine::Events::jPressed(GLFW_KEY_P) && IsEditMode)
            {
                if (selectedOrder.size() != 3)
                    std::cout << "Err: ";
                int i0 = selectedOrder[0];
                int i1 = selectedOrder[1];
                int i2 = selectedOrder[2];

                glm::vec3 A = Engine::Buffers::positions[i0];
                glm::vec3 B = Engine::Buffers::positions[i1];
                glm::vec3 C = Engine::Buffers::positions[i2];

                glm::vec3 n = glm::cross(B - A, C - A);
                float nn = glm::length(n);
                if (nn < 1e-6f)
                    std::cout << "Err: ";
                n /= nn;

                double mx = Engine::Events::x;
                double my = Engine::Events::y;

                glm::mat4 view = cam.view();
                glm::mat4 proj = cam.proj((float)w / (float)h);

                glm::vec2 ndc = Engine::MouseToNDC(mx, my, w, h);
                Engine::Ray ray = Engine::MakeRayFromMouseNDC(ndc, proj, view);

                glm::vec3 hit;
                if (!Engine::RayPlane(ray.origin, ray.dir, A, n, hit))
                    continue;

                RequestPointCreate(hit, PendingPointAction::JustAdd, "P");
            }

            if (Engine::Events::jPressed(GLFW_KEY_V) && IsEditMode && selectedOrder.size() == 1)
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
                int a = selectedOrder[0];
                int b = selectedOrder[1];

                if (Engine::Events::Pressed(GLFW_KEY_LEFT_CONTROL))
                {
                    Engine::Buffers::DisConnectPointsLine(a, b);
                    lineNamesByKey.erase(LineKey(a, b));
                    Engine::Buffers::Update();
                }
                else
                {
                    Engine::Buffers::ConnectPointsLine(a, b);
                    Engine::Buffers::Update();

                    int linesCount = (int)Engine::Buffers::lineIndices.size() / 2;
                    std::string def = std::to_string(linesCount);
                    RequestLineLabel(a, b, def.c_str());
                }
            }
            if (Engine::Events::jPressed(GLFW_KEY_N) && IsEditMode)
            {
                if (selectedOrder.size() == 1)
                {
                    int p = selectedOrder[0];

                    pointNames.resize(Engine::Buffers::positions.size());

                    std::string cur = (p >= 0 && p < (int)pointNames.size()) ? pointNames[p] : "";
                    if (cur.empty())
                        cur = GetPointLabel(p);

                    RequestRenamePoint(p, cur);
                }
                // rename line
                else if (selectedOrder.size() == 2)
                {
                    int a = selectedOrder[0];
                    int b = selectedOrder[1];

                    if (HasLineBetween(a, b))
                    {
                        uint64_t key = LineKey(a, b);

                        std::string cur = "";
                        auto it = lineNamesByKey.find(key);
                        if (it != lineNamesByKey.end())
                            cur = it->second;

                        RequestRenameLine(a, b, cur);
                    }
                }
            }
            // Cutting
            if (Engine::Events::jPressed(GLFW_KEY_S))
            {
                SetSplit();
                IsEditMode = false;
            }

            // Tab
            if (Engine::Events::jPressed(GLFW_KEY_TAB))
            {
                SetEditMode();
            }

            if (Engine::Events::jPressed(GLFW_KEY_G) && IsEditMode && selectedOrder.size() == 4)
            {
                if (!ExtendUsingCutLine())
                {
                    std::cout << "Err: ";
                }
            }

            // if (Engine::Events::jPressed(GLFW_KEY_A) && IsEditMode && selectedOrder.size() == 3)
            // {
            //     Engine::Angle ang(selectedOrder[0], selectedOrder[1], selectedOrder[2]);
            //     ang.active = true;

            //     Engine::AngleUtils::RebuildAngleArc(ang, Engine::Buffers::positions, 40);

            //     angles.push_back(ang);
            // }
        }
        if (Engine::Events::jPressed(GLFW_KEY_E) && IsEditMode &&
            Engine::Events::Pressed(GLFW_KEY_LEFT_CONTROL) &&
            Engine::Buffers::positions.size() == 0)
        {
            if (shape == 8)
            {
                CreateCube();
            }

            if (shape == 108)
            {
                CreateCuboid();
            }

            if (shape == 5)
            {
                CreatePyramid();
            }

            if (shape == 4)
            {
                CreateTetraheadron();
            }

            if (shape == 1)
            {
                CreateCircle(16, 1.0f, 0.0f, 0.0f, 0.0f);
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
            Engine::Buffers::ChangeColor(index, 1.0f, 0.8f, 0.0f, 1.0f);
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
        shaderLines->SetMat4("uView", cam.view());
        shaderLines->SetMat4("uProj", cam.proj(aspect));

        shaderLines->SetVec2("uViewport", glm::vec2((float)Engine::width, (float)Engine::height));
        shaderLines->SetFloat("uDashPx", 10.0f);
        shaderLines->SetFloat("uGapPx", 6.0f);

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

            //////////////////////////////////////////////ImGui::Image((ImTextureID)(intptr_t)my_image_texture, ImVec2(my_image_width, my_image_height));

            ImGui::PushFont(font_bold, 30.00f);
            ImGui::Text("Kuber 3D");
            ImGui::PopFont();

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

            if (ImGui::Button(Engine::Window::IsFullscreen() ? "Exit Fullscreen" : "Fullscreen", ImVec2(-1, 0)))
            {
                Engine::Window::ToggleFullscreen();
            }

            ImGui::PopStyleColor();
            ImGui::SetItemTooltip("F11");

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            if (ImGui::Button("Clean", ImVec2(-1, 0)))
            {
                if (IsEditMode)
                {
                    Engine::Buffers::DeleteAll();
                }
            }

            ImGui::PopStyleColor();
            ImGui::SetItemTooltip("Tap to delete all points (need to be in edit mode)");

            ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

            if (ImGui::BeginTabBar("Select", tab_bar_flags))
            {
                if (ImGui::BeginTabItem("Cube"))
                {
                    ImGui::PopStyleColor();
                    ImGui::Text("Ctrl + E for Cube creation");
                    shape = 8;
                    ImGui::EndTabItem();
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                }
                if (ImGui::BeginTabItem("Pyramid"))
                {
                    ImGui::PopStyleColor();
                    ImGui::Text("Ctrl + E for Pyramid creation");
                    shape = 5;
                    ImGui::EndTabItem();
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                }
                if (ImGui::BeginTabItem("Tetrahedron"))
                {
                    ImGui::PopStyleColor();
                    ImGui::Text("Ctrl + E for Tetrahedron creation");
                    shape = 4;
                    ImGui::EndTabItem();
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                }
                if (ImGui::BeginTabItem("Cuboid"))
                {
                    ImGui::PopStyleColor();
                    ImGui::Text("Ctrl + E for Cuboid creation");
                    shape = 108;
                    ImGui::EndTabItem();
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                }
                if (ImGui::BeginTabItem("Circle"))
                {
                    ImGui::PopStyleColor();
                    ImGui::Text("Ctrl + E for Circle creation");
                    shape = 1;
                    ImGui::EndTabItem();
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                }
                ImGui::EndTabBar();
            }
            ImGui::PopStyleColor();

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
            ImGui::Text("Hover to see tooltips");
            ImGui::PopStyleColor();

            ImGui::Text("TAB to enter in editor mode");
            ImGui::Separator();

            ImGui::Text("V to set view point");
            ImGui::SetItemTooltip("Select 1 point");
            ImGui::Separator();

            ImGui::Text("S to split line");
            ImGui::SetItemTooltip("Select 2 points");
            ImGui::Separator();

            ImGui::Text("P to create point");
            ImGui::SetItemTooltip("Select 3 points");
            ImGui::Separator();

            ImGui::Text("J to create line or to name line");
            ImGui::SetItemTooltip("Select 2 points");
            ImGui::Separator();

            ImGui::Text("Ctrl + J to delete line");
            ImGui::SetItemTooltip("Select 2 points");
            ImGui::Separator();

            ImGui::Text("G to create line in face");
            ImGui::SetItemTooltip("Select 4 points. Begining => direction => line segment (2 points)");
            ImGui::Separator();

            ImGui::Text("R to create perpendicular to plane");
            ImGui::SetItemTooltip("Select 1 point that doesn`t lie in plane => Select 3 points that define a plane");
            ImGui::Separator();

            

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
            // ImGui::Separator();

            ImGui::Text(" %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            if (ImGui::Button("Change theme", ImVec2(-1, 0)))
            {
                Theme = !Theme;
                if (Theme == 1)
                {
                    glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
                    IsEditMode = false;
                }
                else
                {
                    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
                    IsEditMode = false;
                }
            }
            ImGui::PopStyleColor();
            ImGui::End();
        }

        if (Split /*&& IsEditMode*/ && selectedOrder.size() == 2)
        {
            ImGuiWindowFlags flags = /*ImGuiWindowFlags_NoResize |
                                     ImGuiWindowFlags_NoMove |
                                     ImGuiWindowFlags_NoScrollbar |
                                     ImGuiWindowFlags_AlwaysAutoResize |
                                     ImGuiWindowFlags_NoCollapse |
                                     ImGuiWindowFlags_NoFocusOnAppearing |*/
                ImGuiWindowFlags_NoTitleBar;

            ImGui::SetNextWindowPos(ImVec2(Engine::width / 2 - 32, Engine::height / 2 - 64), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(50, -1), ImGuiCond_Always);

            ImGui::Begin("Split", nullptr, flags);
            static char a[5] = "";
            ImGui::InputText("##a", a, IM_ARRAYSIZE(a));
            ImGui::Separator();
            static char b[5] = "";
            ImGui::InputText("##b", b, IM_ARRAYSIZE(b));

            float a1 = static_cast<float>(std::atof(a));
            float b1 = static_cast<float>(std::atof(b));

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            if (ImGui::Button("Split", ImVec2(-1, 0)))
            {
                CutLine(a1, b1);
                Split = false;
                IsEditMode = true;
            }
            ImGui::PopStyleColor();
            ImGui::End();
        }

        glm::mat4 view = cam.view();
        glm::mat4 proj = cam.proj((float)w / (float)h);

        DrawPointLabels(Engine::Buffers::positions, view, proj, w, h);
        DrawPointNamePopup();
        DrawLineNamePopup();
        DrawRenamePopup();
        DrawLineLabels(view, proj, w, h);
        
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
    if (Theme == 1)
    {
        if (!IsEditMode)
        {
            IsEditMode = true;
            glClearColor(0.9f, 0.9f, 1.0f, 1.0f);
        }
        else
        {
            IsEditMode = false;
            glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
        }
    }
    else
    {
        if (!IsEditMode)
        {
            IsEditMode = true;
            glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        }
        else
        {
            IsEditMode = false;
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        }
    }
}

void SetSplit()
{
    if (!Split)
    {
        Split = true;
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
    std::vector<std::string> newNames;
    newNames.reserve(newN);

    for (int i = 0; i < n; i++)
    {
        if (!removed[i])
        {
            if (i < (int)pointNames.size())
                newNames.push_back(pointNames[i]);
            else
                newNames.push_back("");
        }
    }

    pointNames = std::move(newNames);
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

    const int a = selectedOrder[0];
    const int b = selectedOrder[1];

    Engine::Buffers::DisConnectPointsLine(a, b);

    const glm::vec3 A = Engine::Buffers::positions[a];
    const glm::vec3 B = Engine::Buffers::positions[b];

    const glm::vec3 R = A + (B - A) * (p / (p + q));

    RequestPointCreate(R, PendingPointAction::SplitLine, "M", a, b);
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

static bool RayIntersectsSegment3D(const glm::vec3 &A, const glm::vec3 &B, const glm::vec3 &C, const glm::vec3 &D, glm::vec3 &outHit, float eps = 1e-5f)
{
    glm::vec3 R = B - A;
    glm::vec3 S = D - C;
    glm::vec3 W0 = A - C;

    float a = glm::dot(R, R);
    float b = glm::dot(R, S);
    float c = glm::dot(S, S);
    float d = glm::dot(R, W0);
    float e = glm::dot(S, W0);

    if (a <= eps)
    {
        glm::vec3 v = A - C;
        float ss = c;

        if (ss <= eps)
        {

            if (glm::length2(A - C) <= eps * eps)
            {
                outHit = A;
                return true;
            }

            return false;
        }

        float u = glm::dot(v, S) / ss;
        if (u < 0.0f || u > 1.0f)
            return false;

        glm::vec3 Q = C + u * S;

        if (glm::length2(Q - A) <= eps * eps)
        {
            outHit = A;
            return true;
        }
        return false;
    }

    float den = a * c - b * b;

    if (glm::abs(den) <= eps)
    {

        glm::vec3 crossRC = glm::cross((C - A), R);
        if (glm::length2(crossRC) > (eps * eps) * a)
        {
            return false;
        }

        float tC = glm::dot(C - A, R) / a;
        float tD = glm::dot(D - A, R) / a;

        float tMin = glm::min(tC, tD);
        float tMax = glm::max(tC, tD);

        if (tMax < 0.0f)
            return false;

        float tHit = (tMin >= 0.0f) ? tMin : 0.0f;

        if (tHit < tMin - eps || tHit > tMax + eps)
            return false;

        outHit = A + tHit * R;
        return true;
    }

    float t = (b * e - c * d) / den;
    float u = (a * e - b * d) / den;

    if (t < 0.0f)
        return false;
    if (u < 0.0f || u > 1.0f)
        return false;

    glm::vec3 P = A + t * R;
    glm::vec3 Q = C + u * S;

    if (glm::length2(P - Q) > eps * eps)
        return false;

    outHit = (P + Q) * 0.5f;
    return true;
}

bool ExtendUsingCutLine()
{
    if (selectedOrder.size() != 4)
        return false;

    int ia = selectedOrder[0];
    int ib = selectedOrder[1];
    int ic = selectedOrder[2];
    int id = selectedOrder[3];

    if (ia == ib || ia == ic || ia == id || ib == ic || ib == id || ic == id)
        return false;

    glm::vec3 A = Engine::Buffers::positions[ia];
    glm::vec3 B = Engine::Buffers::positions[ib];
    glm::vec3 C = Engine::Buffers::positions[ic];
    glm::vec3 D = Engine::Buffers::positions[id];

    glm::vec3 hit;

    if (!RayIntersectsSegment3D(A, B, C, D, hit, 1e-5f))
        return false;

    Engine::Buffers::AddPoint(hit.x, hit.y, hit.z, pr, pg, pb, pa);

    Engine::Buffers::ConnectPointsLine(ia, ib);
    Engine::Buffers::ConnectPointsLine(ib, Engine::Buffers::positions.size() - 1);

    Engine::Buffers::Update();
    return true;
}

void CreateCuboid()
{
    Engine::Buffers::AddPoint(-1.0f, 0.0f, -1.8f, pr, pg, pb, pa); // 0
    Engine::Buffers::AddPoint(1.0f, 0.0f, -1.8f, pr, pg, pb, pa);  // 1
    Engine::Buffers::AddPoint(-1.0f, 0.0f, 1.8f, pr, pg, pb, pa);  // 2
    Engine::Buffers::AddPoint(1.0f, 0.0f, 1.8f, pr, pg, pb, pa);   // 3

    Engine::Buffers::AddPoint(-1.0f, 2.0f, -1.8f, pr, pg, pb, pa); // 4
    Engine::Buffers::AddPoint(1.0f, 2.0f, -1.8f, pr, pg, pb, pa);  // 5
    Engine::Buffers::AddPoint(-1.0f, 2.0f, 1.8f, pr, pg, pb, pa);  // 6
    Engine::Buffers::AddPoint(1.0f, 2.0f, 1.8f, pr, pg, pb, pa);   // 7

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

void CreateCube()
{
    Engine::Buffers::AddPoint(-1.0f, 0.0f, -1.0f, pr, pg, pb, pa); // 0
    Engine::Buffers::AddPoint(1.0f, 0.0f, -1.0f, pr, pg, pb, pa);  // 1
    Engine::Buffers::AddPoint(-1.0f, 0.0f, 1.0f, pr, pg, pb, pa);  // 2
    Engine::Buffers::AddPoint(1.0f, 0.0f, 1.0f, pr, pg, pb, pa);   // 3

    Engine::Buffers::AddPoint(-1.0f, 2.0f, -1.0f, pr, pg, pb, pa); // 4
    Engine::Buffers::AddPoint(1.0f, 2.0f, -1.0f, pr, pg, pb, pa);  // 5
    Engine::Buffers::AddPoint(-1.0f, 2.0f, 1.0f, pr, pg, pb, pa);  // 6
    Engine::Buffers::AddPoint(1.0f, 2.0f, 1.0f, pr, pg, pb, pa);   // 7

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
    Engine::Buffers::AddPoint(-1.0f, 0.0f, -1.0f, pr, pg, pb, pa); // 0
    Engine::Buffers::AddPoint(1.0f, 0.0f, -1.0f, pr, pg, pb, pa);  // 1
    Engine::Buffers::AddPoint(-1.0f, 0.0f, 1.0f, pr, pg, pb, pa);  // 2
    Engine::Buffers::AddPoint(1.0f, 0.0f, 1.0f, pr, pg, pb, pa);   // 3

    Engine::Buffers::AddPoint(0.0f, 2.0f, 0.0f, pr, pg, pb, pa); // 4

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

    Engine::Buffers::AddQuad(0, 1, 3, 2);

    Engine::Buffers::AddFace(0, 1, 4);

    Engine::Buffers::AddFace(1, 3, 4);

    Engine::Buffers::AddFace(3, 2, 4);

    Engine::Buffers::AddFace(2, 0, 4);

    Engine::Buffers::Update();
}

void CreateTetraheadron()
{
    Engine::Buffers::AddPoint(-1.0f, 0.0f, -0.8f, pr, pg, pb, pa); // 0
    Engine::Buffers::AddPoint(1.0f, 0.0f, -0.8f, pr, pg, pb, pa);  // 1
    Engine::Buffers::AddPoint(0.0f, 0.0f, 1.3f, pr, pg, pb, pa);   // 2

    Engine::Buffers::AddPoint(0.0f, 2.0f, 0.0f, pr, pg, pb, pa); // 3

    // 0
    Engine::Buffers::ConnectPointsLine(0, 1);
    Engine::Buffers::ConnectPointsLine(0, 2);
    Engine::Buffers::ConnectPointsLine(0, 3);
    // 1
    Engine::Buffers::ConnectPointsLine(1, 2);
    Engine::Buffers::ConnectPointsLine(1, 3);
    // 3
    Engine::Buffers::ConnectPointsLine(3, 2);

    Engine::Buffers::AddFace(0, 1, 2);

    Engine::Buffers::AddFace(0, 1, 3);

    Engine::Buffers::AddFace(1, 2, 3);

    Engine::Buffers::AddFace(2, 0, 3);

    Engine::Buffers::Update();
}

void CreateCircle(int N, float R, float cx, float cy, float cz)
{
    const float TWO_PI = 6.283185307179586f;

    for (int i = 0; i < N; ++i)
    {
        float t = TWO_PI * (float)i / (float)N;

        float x = cx + R * std::sin(t);
        float y = cy;
        float z = cz + R * std::cos(t);

        Engine::Buffers::AddPoint(x, y, z, pr, pg, pb, pa);

        for (int pt_b = 0; pt_b < N; pt_b++)
        {
            Engine::Buffers::ConnectPointsLine(pt_b, pt_b + 1);
            Engine::Buffers::ConnectPointsLine(N - 1, 0);
        }
    }
}
// INCORRECT 
void PerpToPlane()
{
    int id = selectedOrder[0];
    int ia = selectedOrder[1];
    int ib = selectedOrder[2];
    int ic = selectedOrder[3];

    glm::vec3 D = Engine::Buffers::positions[id];
    glm::vec3 A = Engine::Buffers::positions[ia];
    glm::vec3 B = Engine::Buffers::positions[ib];
    glm::vec3 C = Engine::Buffers::positions[ic];

    glm::vec3 u = B - A;
    glm::vec3 v = C - A;

    glm::vec3 n = glm::cross(u, v);

    float nn = glm::dot(n, n);
    if (nn < 1e-8f)
        return;

    glm::vec3 w = D - A;

    float t = glm::dot(w, n) / nn;

    glm::vec3 perp = t * n;

    glm::vec3 H = D - perp;

    float dist = glm::length(perp);

    RequestPointCreate(H, PendingPointAction::PerpToPlane, "H", -1, -1, -1, -1, id);
}

std::string GetPointLabel(int i)
{
    static const char *k8[] = {"A", "B", "C", "D", "A1", "B1", "C1", "D1"};
    if (i >= 0 && i < 8)
        return k8[i];

    std::string res = (i == 8) ? "P" : "P" + std::to_string(i - 8);
    return res;
}

void DrawPointLabels(
    const std::vector<glm::vec3> &positions,
    const glm::mat4 &view,
    const glm::mat4 &proj,
    int w, int h)
{
    ImDrawList *dl = ImGui::GetBackgroundDrawList();
    if (!dl)
        return;

    ImU32 col = (Theme == 0) ? IM_COL32(255, 255, 255, 255) : IM_COL32(0, 71, 171, 255);
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

void DrawPointNamePopup()
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
        Engine::Buffers::AddPoint(gPendingPoint.pos.x, gPendingPoint.pos.y, gPendingPoint.pos.z, pr, pg, pb, pa);

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

void RequestPointCreate(
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

void DrawLineLabels(
    const glm::mat4 &view,
    const glm::mat4 &proj,
    int w, int h)
{
    ImDrawList *dl = ImGui::GetBackgroundDrawList();
    if (!dl)
        return;

    ImU32 col = (Theme == 0) ? IM_COL32(255, 255, 255, 255) : IM_COL32(0, 71, 171, 255);
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

void RequestLineLabel(int a, int b, const char *defaultName)
{
    gPendingLine.active = true;
    gPendingLine.a = a;
    gPendingLine.b = b;
    std::snprintf(gPendingLine.name, sizeof(gPendingLine.name), "%s", defaultName);
}

void DrawLineNamePopup()
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

bool HasLineBetween(int a, int b)
{
    auto &L = Engine::Buffers::lineIndices;
    for (int i = 0; i + 1 < (int)L.size(); i += 2)
    {
        int x = (int)L[i];
        int y = (int)L[i + 1];
        if ((x == a && y == b) || (x == b && y == a))
            return true;
    }
    return false;
}

void DrawRenamePopup()
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

void RequestRenamePoint(int idx, const std::string &current)
{
    gRename.active = true;
    gRename.mode = RenameMode::Point;
    gRename.pointIndex = idx;
    gRename.a = gRename.b = -1;
    std::snprintf(gRename.name, sizeof(gRename.name), "%s", current.c_str());
}

void RequestRenameLine(int a, int b, const std::string &current)
{
    gRename.active = true;
    gRename.mode = RenameMode::Line;
    gRename.pointIndex = -1;
    gRename.a = a;
    gRename.b = b;
    std::snprintf(gRename.name, sizeof(gRename.name), "%s", current.c_str());
}
