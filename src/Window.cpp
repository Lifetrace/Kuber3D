#include "Window.hpp"
#include "GLincs.hpp"
#include <iostream>

GLFWwindow* Engine::Window::window = nullptr;

static bool g_isFullscreen = false;
static int g_windowedX = 100;
static int g_windowedY = 100;
static int g_windowedW = 1280;
static int g_windowedH = 720;

namespace Engine {
    int width  = 1280;
    int height = 720;
}

void frame_buffersize_callback(GLFWwindow* window, int Width, int Height){
    Engine::width = Width;
    Engine::height = Height;
    glViewport(0, 0, Width, Height);
}

int Engine::Window::Init(int width, int height, const char* title){
    if(!glfwInit()){
        std::cerr << "Can`t init program!" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 8);

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);

    if(!window){
        std::cerr << "Can`t create window or window is null" << std::endl;
        Engine::Window::Terminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetFramebufferSizeCallback(window, frame_buffersize_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to init GLAD\n";
        Engine::Window::Terminate();
        return -1;
    }

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glViewport(0, 0, width, height);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return 0;
}

void Engine::Window::Terminate(){
    if(window){
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

void Engine::Window::SwapBuffers(GLFWwindow* window){
    glfwSwapBuffers(window);
}

bool Engine::Window::isShouldClose(GLFWwindow* window){
    return glfwWindowShouldClose(window);
}

void Engine::Window::SetClose(GLFWwindow* window, bool state){
    glfwSetWindowShouldClose(window, state);
}

GLFWwindow* Engine::Window::GetWin(){
    return window;
}

void Engine::Window::ToggleFullscreen()
{
    if (!window) return;

    g_isFullscreen = !g_isFullscreen;

    if (g_isFullscreen)
    {
        glfwGetWindowPos(window, &g_windowedX, &g_windowedY);
        glfwGetWindowSize(window, &g_windowedW, &g_windowedH);

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        if (!monitor) return;

        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        if (!mode) return;

        glfwSetWindowMonitor(
            window,
            monitor,
            0, 0,
            mode->width,
            mode->height,
            mode->refreshRate
        );

        Engine::width = mode->width;
        Engine::height = mode->height;
        glViewport(0, 0, Engine::width, Engine::height);
    }
    else
    {
        glfwSetWindowMonitor(
            window,
            nullptr,
            g_windowedX,
            g_windowedY,
            g_windowedW,
            g_windowedH,
            0
        );

        Engine::width = g_windowedW;
        Engine::height = g_windowedH;
        glViewport(0, 0, Engine::width, Engine::height);
    }
}

bool Engine::Window::IsFullscreen()
{
    return g_isFullscreen;
}