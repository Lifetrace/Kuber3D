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
#include "Operations.hpp"
#include "Figures.hpp"

#include "Selecting.hpp"

#include "VectorMath.hpp"

#include "PointActions.hpp"

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
bool IsEditMode = false;
bool Split = false;
int shape = 8;

// Angles
std::vector<Engine::Angle> angles; // in dev

// Camera
Engine::Camera cam;
glm::vec3 targetH = cam.target;

// Set Edit Mode
void SetEditMode();

// Operations
void SetSplit();
bool HasLineBetween(int a, int b);

// Letters
static void DrawPointLabels(
    const std::vector<glm::vec3> &positions,
    const glm::mat4 &view,
    const glm::mat4 &proj,
    int w, int h);

// Future
int IndexByPos(const glm::vec3 &pos);

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

    const bool uiCapturesMouse =
        io.WantCaptureMouse ||
        Engine::PointActions::gPendingPoint.active ||
        Engine::PointActions::gPendingLine.active ||
        Engine::PointActions::gRename.active ||
        Split;

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

    Engine::Buffers::AddPoint(0.0f, 0.0f, 0.0f, Engine::Buffers::pr, Engine::Buffers::pg, Engine::Buffers::pb, Engine::Buffers::pa);
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

        const bool blockHotkeys = uiCapturesKeyboard || Engine::PointActions::gPendingPoint.active || Split;

        if (Engine::Style::Theme == 1)
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

            if (Engine::Events::jPressed(GLFW_KEY_R) && IsEditMode && Engine::Selecting::selectedOrder.size() == 4)
            {
                Engine::Operations::PerpToPlane();
            }

            if (Engine::Events::jPressed(GLFW_KEY_P) && IsEditMode)
            {
                if (Engine::Selecting::selectedOrder.size() != 3)
                {
                    std::cout << "Err: select exactly 3 points for P\n";
                }
                else
                {
                    int i0 = Engine::Selecting::selectedOrder[0];
                    int i1 = Engine::Selecting::selectedOrder[1];
                    int i2 = Engine::Selecting::selectedOrder[2];

                    glm::vec3 A = Engine::Buffers::positions[i0];
                    glm::vec3 B = Engine::Buffers::positions[i1];
                    glm::vec3 C = Engine::Buffers::positions[i2];

                    glm::vec3 n = glm::cross(B - A, C - A);
                    float nn = glm::length(n);
                    if (nn < 1e-6f)
                    {
                        std::cout << "Err: degenerate plane\n";
                    }
                    else
                    {
                        n /= nn;

                        double mx = Engine::Events::x;
                        double my = Engine::Events::y;

                        glm::mat4 view = cam.view();
                        glm::mat4 proj = cam.proj((float)w / (float)h);

                        glm::vec2 ndc = Engine::MouseToNDC(mx, my, w, h);
                        Engine::Ray ray = Engine::MakeRayFromMouseNDC(ndc, proj, view);

                        glm::vec3 hit;
                        if (Engine::RayPlane(ray.origin, ray.dir, A, n, hit))
                        {
                            Engine::PointActions::RequestPointCreate(hit, Engine::PointActions::PendingPointAction::JustAdd, "P");
                        }
                    }
                }
            }

            if (Engine::Events::jPressed(GLFW_KEY_V) && IsEditMode && Engine::Selecting::selectedOrder.size() == 1)
            {
                int idx = Engine::Selecting::selectedOrder[0];
                targetH = Engine::Buffers::positions[idx];
            }

            // DeletePoints
            if (Engine::Events::jPressed(GLFW_KEY_DELETE) && IsEditMode)
            {
                Engine::Selecting::DelAllSelected();
            }

            // ConnectPoints
            if (Engine::Events::jPressed(GLFW_KEY_J) && IsEditMode && Engine::Selecting::selectedOrder.size() == 2)
            {
                int a = Engine::Selecting::selectedOrder[0];
                int b = Engine::Selecting::selectedOrder[1];

                if (Engine::Events::Pressed(GLFW_KEY_LEFT_CONTROL))
                {
                    Engine::Buffers::DisConnectPointsLine(a, b);
                    Engine::PointActions::lineNamesByKey.erase(Engine::PointActions::LineKey(a, b));
                    Engine::Buffers::Update();
                }
                else
                {
                    Engine::Buffers::ConnectPointsLine(a, b);
                    Engine::Buffers::Update();

                    int linesCount = (int)Engine::Buffers::lineIndices.size() / 2;
                    std::string def = std::to_string(linesCount);
                    Engine::PointActions::RequestLineLabel(a, b, def.c_str());
                }
            }
            if (Engine::Events::jPressed(GLFW_KEY_N) && IsEditMode)
            {
                if (Engine::Selecting::selectedOrder.size() == 1)
                {
                    int p = Engine::Selecting::selectedOrder[0];

                    Engine::PointActions::pointNames.resize(Engine::Buffers::positions.size());

                    std::string cur = (p >= 0 && p < (int)Engine::PointActions::pointNames.size()) ? Engine::PointActions::pointNames[p] : "";
                    if (cur.empty())
                        cur = Engine::GetPointLabel(p);

                    Engine::PointActions::RequestRenamePoint(p, cur);
                }
                // rename line
                else if (Engine::Selecting::selectedOrder.size() == 2)
                {
                    int a = Engine::Selecting::selectedOrder[0];
                    int b = Engine::Selecting::selectedOrder[1];

                    if (HasLineBetween(a, b))
                    {
                        uint64_t key = Engine::PointActions::LineKey(a, b);

                        std::string cur = "";
                        auto it = Engine::PointActions::lineNamesByKey.find(key);
                        if (it != Engine::PointActions::lineNamesByKey.end())
                            cur = it->second;

                        Engine::PointActions::RequestRenameLine(a, b, cur);
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

            if (Engine::Events::jPressed(GLFW_KEY_G) && IsEditMode && Engine::Selecting::selectedOrder.size() == 4)
            {
                if (!Engine::Operations::ExtendUsingCutLine())
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
                Engine::Figures::CreateCube();
            }

            if (shape == 108)
            {
                Engine::Figures::CreateCuboid();
            }

            if (shape == 5)
            {
                Engine::Figures::CreatePyramid();
            }

            if (shape == 4)
            {
                Engine::Figures::CreateTetrahedron();
            }

            if (shape == 1)
            {
                Engine::Figures::CreateCircle(16, 1.0f, 0.0f, 0.0f, 0.0f);
            }
        }

        // Navigation
        if (!uiCapturesMouse && Engine::Events::jclicked(GLFW_MOUSE_BUTTON_LEFT) && IsEditMode)
        {
            if (!Engine::Events::Pressed(GLFW_KEY_LEFT_SHIFT))
                Engine::Selecting::DesAllSelected();

            double mx = Engine::Events::x;
            double my = Engine::Events::y;

            glm::vec2 ndc = Engine::MouseToNDC(mx, my, w, h);

            glm::mat4 view = cam.view();
            glm::mat4 proj = cam.proj((float)w / (float)h);

            Engine::Ray ray = Engine::MakeRayFromMouseNDC(ndc, proj, view);

            int idx = Engine::VectorMath::PickPointByPixels(Engine::Buffers::positions, mx, my, view, proj, w, h, 10.0f);

            if (idx != -1)
            {
                Engine::Selecting::ToggleSelect(idx);
            }
        }

        // Shader Use and Set Matrixes
        shaderBase->use();
        shaderBase->SetMat4("uModel", model);
        shaderBase->SetMat4("uView", cam.view());
        shaderBase->SetMat4("uProj", cam.proj(aspect));

        // Color Selected Points
        for (const auto &[index, col] : Engine::Selecting::selectedPointsByColor)
        {
            Engine::Buffers::ChangeColor(index, 1.0f, 0.8f, 0.0f, 1.0f);
        }

        // Defense
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
                Engine::Style::Theme = !Engine::Style::Theme;
                if (Engine::Style::Theme == 1)
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

            if (ImGui::Button("Exit", ImVec2(-1, 0)))
            {
                Engine::Window::SetClose(Engine::Window::GetWin(), true);
            }
            
            ImGui::PopStyleColor();
            ImGui::End();
        }

        if (Split /*&& IsEditMode*/ && Engine::Selecting::selectedOrder.size() == 2)
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
                Engine::Operations::CutLine(a1, b1);
                Split = false;
                IsEditMode = true;
            }
            ImGui::PopStyleColor();
            ImGui::End();
        }

        glm::mat4 view = cam.view();
        glm::mat4 proj = cam.proj((float)w / (float)h);

        Engine::PointActions::DrawPointLabels(Engine::Buffers::positions, view, proj, w, h);
        Engine::PointActions::DrawPointNamePopup();
        Engine::PointActions::DrawLineNamePopup();
        Engine::PointActions::DrawRenamePopup();
        Engine::PointActions::DrawLineLabels(view, proj, w, h);

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
    if (Engine::Style::Theme == 1)
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
