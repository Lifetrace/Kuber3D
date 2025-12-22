#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Engine.hpp"
#include <iostream>

int main(){
    Engine::Window::Init(800, 600, "Window");
    
    while (!Engine::Window::isShouldClose(Engine::Window::window))
    {
        Engine::Window::SwapBuffers(Engine::Window::window);
        Engine::Events::PollEvents();
    }
    
    glfwTerminate();
    return 0;
}