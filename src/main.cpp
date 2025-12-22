#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Engine.hpp"
#include "Events.hpp"
#include <iostream>

const float color[4] = {0.15f, 0.15f, 0.15f, 1.0f};

int main(){
    Engine::Window::Init(800, 600, "Window", color);
    
    while (!Engine::Window::isShouldClose(Engine::Window::window))
    {
        Engine::Window::ClearScreen(GL_COLOR_BUFFER_BIT);
        
        Engine::Window::SwapBuffers(Engine::Window::window);
        Engine::Events::PollEvents();
    }

    Engine::Window::Terminate();
    return 0;
}