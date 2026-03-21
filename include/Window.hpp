#pragma once 

class GLFWwindow;

namespace Engine{
    extern int width;
    extern int height;
    class Window{
    public:

        static GLFWwindow *window;
        static int  Init(int width, int height, const char* title);
        static void Terminate();
        static void SwapBuffers(GLFWwindow* window);
        static bool isShouldClose(GLFWwindow* window);
        static void SetClose(GLFWwindow* window, bool state);
        static GLFWwindow* GetWin();
        static void ToggleFullscreen();
        static bool IsFullscreen();
    };
}