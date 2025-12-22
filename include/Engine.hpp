#pragma once

class GLFWwindow;

namespace Engine {
    class Window {
    public:
        static GLFWwindow* window;
        static void Init(int width, int hight, const char* title);
        static bool isShouldClose(GLFWwindow* window);
        static void SwapBuffers(GLFWwindow* window);
    };
    class Events{
    public:
        static void PollEvents();
    };
};