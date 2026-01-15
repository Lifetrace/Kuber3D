#include "Events.hpp"
#include <algorithm>

Engine::Events& Engine::Events::Get() {
    static Events instance;
    return instance;
}

void Engine::Events::Init(GLFWwindow* window) {
    _window = window;

    glfwSetWindowUserPointer(_window, this);

    glfwSetKeyCallback(_window, &Events::KeyCallback);
    glfwSetCharCallback(_window, &Events::CharCallback);
    glfwSetMouseButtonCallback(_window, &Events::MouseButtonCallback);
    glfwSetCursorPosCallback(_window, &Events::CursorPosCallback);
    glfwSetScrollCallback(_window, &Events::ScrollCallback);
    glfwSetFramebufferSizeCallback(_window, &Events::FramebufferSizeCallback);

    glfwGetFramebufferSize(_window, &_width, &_height);

    glfwGetCursorPos(_window, &_mouseX, &_mouseY);
    _mouseInited = true;

    _keyDown.fill(false);
    _keyPrev.fill(false);
    _keyPressed.fill(false);
    _keyReleased.fill(false);

    _mouseDown.fill(false);
    _mousePrev.fill(false);
    _mousePressed.fill(false);
    _mouseReleased.fill(false);

    _mouseDX = _mouseDY = 0.0;
    _mouseDXAccum = _mouseDYAccum = 0.0;

    _scrollX = _scrollY = 0.0;
    _scrollXAccum = _scrollYAccum = 0.0;
}

void Engine::Events::Update() {
    for (int k = 0; k <= KEY_MAX; ++k) {
        const bool cur = _keyDown[k];
        const bool prev = _keyPrev[k];
        _keyPressed[k]  = (cur && !prev);
        _keyReleased[k] = (!cur && prev);
        _keyPrev[k] = cur;
    }

    for (int b = 0; b <= MOUSE_MAX; ++b) {
        const bool cur = _mouseDown[b];
        const bool prev = _mousePrev[b];
        _mousePressed[b]  = (cur && !prev);
        _mouseReleased[b] = (!cur && prev);
        _mousePrev[b] = cur;
    }

    _mouseDX = _mouseDXAccum;
    _mouseDY = _mouseDYAccum;
    _mouseDXAccum = 0.0;
    _mouseDYAccum = 0.0;

    _scrollX = _scrollXAccum;
    _scrollY = _scrollYAccum;
    _scrollXAccum = 0.0;
    _scrollYAccum = 0.0;
}

void Engine::Events::PollEvents(){
    glfwPollEvents();
}

bool Engine::Events::IsKeyDown(int key) const {
    if (key < 0 || key > KEY_MAX) return false;
    return _keyDown[key];
}
bool Engine::Events::IsKeyPressed(int key) const {
    if (key < 0 || key > KEY_MAX) return false;
    return _keyPressed[key];
}
bool Engine::Events::IsKeyReleased(int key) const {
    if (key < 0 || key > KEY_MAX) return false;
    return _keyReleased[key];
}

bool Engine::Events::IsMouseDown(int button) const {
    if (button < 0 || button > MOUSE_MAX) return false;
    return _mouseDown[button];
}
bool Engine::Events::IsMousePressed(int button) const {
    if (button < 0 || button > MOUSE_MAX) return false;
    return _mousePressed[button];
}
bool Engine::Events::IsMouseReleased(int button) const {
    if (button < 0 || button > MOUSE_MAX) return false;
    return _mouseReleased[button];
}

std::size_t Engine::Events::OnKey(const KeyFn& fn) {
    _onKey.push_back(fn);
    return _onKey.size() - 1;
}
std::size_t Engine::Events::OnChar(const CharFn& fn) {
    _onChar.push_back(fn);
    return _onChar.size() - 1;
}
std::size_t Engine::Events::OnMouseButton(const MouseBtnFn& fn) {
    _onMouseButton.push_back(fn);
    return _onMouseButton.size() - 1;
}
std::size_t Engine::Events::OnMouseMove(const MouseMoveFn& fn) {
    _onMouseMove.push_back(fn);
    return _onMouseMove.size() - 1;
}
std::size_t Engine::Events::OnScroll(const ScrollFn& fn) {
    _onScroll.push_back(fn);
    return _onScroll.size() - 1;
}
std::size_t Engine::Events::OnResize(const ResizeFn& fn) {
    _onResize.push_back(fn);
    return _onResize.size() - 1;
}

static Engine::Events* Self(GLFWwindow* w) {
    return reinterpret_cast<Engine::Events*>(glfwGetWindowUserPointer(w));
}

void Engine::Events::KeyCallback(GLFWwindow* w, int key, int scancode, int action, int mods) {
    if (auto* self = Self(w)) self->HandleKey(key, scancode, action, mods);
}
void Engine::Events::CharCallback(GLFWwindow* w, unsigned int codepoint) {
    if (auto* self = Self(w)) self->HandleChar(codepoint);
}
void Engine::Events::MouseButtonCallback(GLFWwindow* w, int button, int action, int mods) {
    if (auto* self = Self(w)) self->HandleMouseButton(button, action, mods);
}
void Engine::Events::CursorPosCallback(GLFWwindow* w, double x, double y) {
    if (auto* self = Self(w)) self->HandleCursorPos(x, y);
}
void Engine::Events::ScrollCallback(GLFWwindow* w, double xoffset, double yoffset) {
    if (auto* self = Self(w)) self->HandleScroll(xoffset, yoffset);
}
void Engine::Events::FramebufferSizeCallback(GLFWwindow* w, int width, int height) {
    if (auto* self = Self(w)) self->HandleResize(width, height);
}

void Engine::Events::HandleKey(int key, int scancode, int action, int mods) {
    if (key >= 0 && key <= KEY_MAX) {
        if (action == GLFW_PRESS) _keyDown[key] = true;
        else if (action == GLFW_RELEASE) _keyDown[key] = false;
        else if (action == GLFW_REPEAT) _keyDown[key] = true; 
    }

    for (auto& fn : _onKey) {
        if (fn) fn(key, scancode, action, mods);
    }
}

void Engine::Events::HandleChar(unsigned int codepoint) {
    for (auto& fn : _onChar) {
        if (fn) fn(codepoint);
    }
}

void Engine::Events::HandleMouseButton(int button, int action, int mods) {
    if (button >= 0 && button <= MOUSE_MAX) {
        if (action == GLFW_PRESS) _mouseDown[button] = true;
        else if (action == GLFW_RELEASE) _mouseDown[button] = false;
    }

    for (auto& fn : _onMouseButton) {
        if (fn) fn(button, action, mods);
    }
}

void Engine::Events::HandleCursorPos(double x, double y) {
    if (!_mouseInited) {
        _mouseX = x; _mouseY = y;
        _mouseInited = true;
        return;
    }

    const double dx = x - _mouseX;
    const double dy = y - _mouseY;

    _mouseX = x;
    _mouseY = y;

    _mouseDXAccum += dx;
    _mouseDYAccum += dy;

    for (auto& fn : _onMouseMove) {
        if (fn) fn(_mouseX, _mouseY, dx, dy);
    }
}

void Engine::Events::HandleScroll(double xoffset, double yoffset) {
    _scrollXAccum += xoffset;
    _scrollYAccum += yoffset;

    for (auto& fn : _onScroll) {
        if (fn) fn(xoffset, yoffset);
    }
}

void Engine::Events::HandleResize(int width, int height) {
    _width = width;
    _height = height;

    glViewport(0, 0, width, height);

    for (auto& fn : _onResize) {
        if (fn) fn(width, height);
    }
}
