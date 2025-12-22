#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Engine.hpp"
#include <iostream>

GLFWwindow* Engine::Window::window = nullptr;

void Engine::Window::Init(int width, int height, const char* title){

    if (!glfwInit()){
        std::cout << "Can`t initializate GLFW!" << std::endl;
    }

    Window::window = glfwCreateWindow(width, height, title, NULL, NULL);
    
    if (!window){
        std::cout << "Can`t create window, or window is NULL" << std::endl;
        glfwTerminate();
    }
    
    glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "Can`t load GLAD" << std::endl;
        glfwTerminate();
    }

    glViewport(0, 0, width, height);
}

bool Engine::Window::isShouldClose(GLFWwindow* window){
    return glfwWindowShouldClose(window);
}

void Engine::Window::SwapBuffers(GLFWwindow* window){
    glfwSwapBuffers(window);
}

void Engine::Events::PollEvents(){
    glfwPollEvents();
}