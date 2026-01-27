#include "Events.hpp"
#include "Window.hpp"
#include "GLincs.hpp"
#include <string.h>  

bool* Engine::Events::_keys;
uint* Engine::Events::_frames;
uint Engine::Events::_current = 0;
float Engine::Events::deltaX = 0.0f;
float Engine::Events::deltaY = 0.0f;
float Engine::Events::x = 0.0f;
float Engine::Events::y = 0.0f;
bool Engine::Events::_cursor_locked = false;
bool Engine::Events::_cursor_started = false;


void cursor_position_callback(GLFWwindow* window, double xpos, double ypos){
    if (Engine::Events::_cursor_started){
        Engine::Events::deltaX += xpos-Engine::Events::x;
        Engine::Events::deltaY += ypos-Engine::Events::y;
    }else{
        Engine::Events::_cursor_started = true;
    }
    Engine::Events::x = xpos;
    Engine::Events::y = ypos;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mode)
{
    static constexpr int KEY_MAX   = GLFW_KEY_LAST + 1;
    static constexpr int MOUSE_MAX = GLFW_MOUSE_BUTTON_LAST + 1;

    if (button < 0 || button >= MOUSE_MAX) return;

    int idx = KEY_MAX + button;

    if (action == GLFW_PRESS)
    {
        Engine::Events::_keys[idx] = true;
        Engine::Events::_frames[idx] = Engine::Events::_current;
    }
    else if (action == GLFW_RELEASE)
    {
        Engine::Events::_keys[idx] = false;
    }
}



void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode){
    if (action == GLFW_PRESS){
        Engine::Events::_keys[key] = true;
        Engine::Events::_frames[key] = Engine::Events::_current;

    } else if (action == GLFW_RELEASE){
        Engine::Events::_keys[key] = false;
        Engine::Events::_frames[key] = Engine::Events::_current;
    }
}

int Engine::Events::initialize(){
    GLFWwindow* window = Engine::Window::GetWin();

    static constexpr int KEY_MAX   = GLFW_KEY_LAST + 1;
    static constexpr int MOUSE_MAX = GLFW_MOUSE_BUTTON_LAST + 1;
    static constexpr int OFFSET_MOUSE = KEY_MAX;
    static constexpr int TOTAL = KEY_MAX + MOUSE_MAX;

    _keys   = new bool[TOTAL];
    _frames = new uint[TOTAL];

    memset(_keys,   0, TOTAL * sizeof(bool));
    memset(_frames, 0, TOTAL * sizeof(uint));

    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    
    return 0;
}


bool Engine::Events::Pressed(int keycode){
    static constexpr int KEY_MAX = GLFW_KEY_LAST + 1;
    if (keycode < 0 || keycode >= KEY_MAX) return false;
    return _keys[keycode];
}

bool Engine::Events::jPressed(int keycode){
    static constexpr int KEY_MAX = GLFW_KEY_LAST + 1;
    if (keycode < 0 || keycode >= KEY_MAX) return false;
    return _keys[keycode] && _frames[keycode] == _current;
}

bool Engine::Events::clicked(int button){
    static constexpr int KEY_MAX   = GLFW_KEY_LAST + 1;
    static constexpr int MOUSE_MAX = GLFW_MOUSE_BUTTON_LAST + 1;
    if (button < 0 || button >= MOUSE_MAX) return false;
    return _keys[KEY_MAX + button];
}

bool Engine::Events::jclicked(int button){
    static constexpr int KEY_MAX   = GLFW_KEY_LAST + 1;
    static constexpr int MOUSE_MAX = GLFW_MOUSE_BUTTON_LAST + 1;
    if (button < 0 || button >= MOUSE_MAX) return false;
    int idx = KEY_MAX + button;
    return _keys[idx] && _frames[idx] == _current;
}


void Engine::Events::PollEvents(){
    _current++;
    deltaX = 0.0f;
    deltaY = 0.0f;
    glfwPollEvents();
}