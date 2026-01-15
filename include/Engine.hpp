#pragma once

class GLFWwindow;

namespace Engine {
    class Window {
    public:
        static GLFWwindow* window;
        static void Init(int width, int hight, const char* title, const float color[4]);
        static void Terminate();
        static bool isShouldClose(GLFWwindow* window);
        static void SwapBuffers(GLFWwindow* window);
        static void ClearScreen(GLbitfield mask);
        static float GetTime();
    };
}