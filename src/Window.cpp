#include "Window.hpp"
#include "GLincs.hpp"
#include <iostream>

GLFWwindow* Engine::Window::window = nullptr;

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

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to init GLAD\n";
        Engine::Window::Terminate();
        return -1;
    }

    glEnable(GL_PROGRAM_POINT_SIZE);
    glViewport(0, 0, width, height);
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