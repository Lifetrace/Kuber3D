#pragma once 

class GLFWwindow;

namespace Engine{
    class Window{
        static GLFWwindow *window;
    public:
        static int  Init(int width, int height, const char* title);
        static void Terminate();
        static void SwapBuffers(GLFWwindow* window);
        static bool isShouldClose(GLFWwindow* window);
        static void SetClose(GLFWwindow* window, bool state);
        static GLFWwindow* GetWin();
    };
}