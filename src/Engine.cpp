#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Engine.hpp"
#include <iostream>

GLFWwindow* Engine::Window::window = nullptr;

void Engine::Window::Init(int width, int height, const char* title, const float color[4]){

    if (!glfwInit()){
        std::cout << "Can`t initializate GLFW!" << std::endl;
    }

    Window::window = glfwCreateWindow(width, height, title, NULL, NULL);
    
    if (!window){
        std::cout << "Can`t create window, or window is NULL" << std::endl;
        Window::Terminate();
    }
    
    glfwMakeContextCurrent(window);
    
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "Can`t load GLAD" << std::endl;
        Window::Terminate();
    }
    glClearColor(color[0], color[1], color[2], color[3]);
    glViewport(0, 0, width, height);
}

bool Engine::Window::isShouldClose(GLFWwindow* window){
    return glfwWindowShouldClose(window);
}

void Engine::Window::SwapBuffers(GLFWwindow* window){
    glfwSwapBuffers(window);
}

void Engine::Window::ClearScreen(GLbitfield mask){
    glClear(mask);
}

void Engine::Window::Terminate(){
    if(window){
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

float Engine::Window::GetTime(){
    return (float)glfwGetTime();
}