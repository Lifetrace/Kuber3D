#include "Events.hpp"
#include <GLFW/glfw3.h>

void Engine::Events::PollEvents(){
    glfwPollEvents();
}