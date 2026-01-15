#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <array>
#include <vector>
#include <functional>
#include <cstddef>
namespace Engine{
    class Events {
    public:
        using KeyFn        = std::function<void(int key, int scancode, int action, int mods)>;
        using CharFn       = std::function<void(unsigned int codepoint)>;
        using MouseBtnFn   = std::function<void(int button, int action, int mods)>;
        using MouseMoveFn  = std::function<void(double x, double y, double dx, double dy)>;
        using ScrollFn     = std::function<void(double xoffset, double yoffset)>;
        using ResizeFn     = std::function<void(int width, int height)>;

        static Events& Get();
        static void PollEvents();

        void Init(GLFWwindow* window);

        void Update();


        bool IsKeyDown(int key) const;
        bool IsKeyPressed(int key) const;  
        bool IsKeyReleased(int key) const;  

        bool IsMouseDown(int button) const;
        bool IsMousePressed(int button) const;
        bool IsMouseReleased(int button) const;

        double MouseX() const { return _mouseX; }
        double MouseY() const { return _mouseY; }
        double MouseDX() const { return _mouseDX; } 
        double MouseDY() const { return _mouseDY; } 

        double ScrollX() const { return _scrollX; }
        double ScrollY() const { return _scrollY; }

        int Width()  const { return _width; }
        int Height() const { return _height; }

        GLFWwindow* Window() const { return _window; }

        std::size_t OnKey(const KeyFn& fn);
        std::size_t OnChar(const CharFn& fn);
        std::size_t OnMouseButton(const MouseBtnFn& fn);
        std::size_t OnMouseMove(const MouseMoveFn& fn);
        std::size_t OnScroll(const ScrollFn& fn);
        std::size_t OnResize(const ResizeFn& fn);

    private:
        Events() = default;

        static void KeyCallback(GLFWwindow* w, int key, int scancode, int action, int mods);
        static void CharCallback(GLFWwindow* w, unsigned int codepoint);
        static void MouseButtonCallback(GLFWwindow* w, int button, int action, int mods);
        static void CursorPosCallback(GLFWwindow* w, double x, double y);
        static void ScrollCallback(GLFWwindow* w, double xoffset, double yoffset);
        static void FramebufferSizeCallback(GLFWwindow* w, int width, int height);

        void HandleKey(int key, int scancode, int action, int mods);
        void HandleChar(unsigned int codepoint);
        void HandleMouseButton(int button, int action, int mods);
        void HandleCursorPos(double x, double y);
        void HandleScroll(double xoffset, double yoffset);
        void HandleResize(int width, int height);

    private:
        GLFWwindow* _window = nullptr;

        static constexpr int KEY_MAX    = GLFW_KEY_LAST;
        static constexpr int MOUSE_MAX  = GLFW_MOUSE_BUTTON_LAST;

        std::array<bool, KEY_MAX + 1>   _keyDown{};
        std::array<bool, KEY_MAX + 1>   _keyPrev{};
        std::array<bool, KEY_MAX + 1>   _keyPressed{};
        std::array<bool, KEY_MAX + 1>   _keyReleased{};

        std::array<bool, MOUSE_MAX + 1> _mouseDown{};
        std::array<bool, MOUSE_MAX + 1> _mousePrev{};
        std::array<bool, MOUSE_MAX + 1> _mousePressed{};
        std::array<bool, MOUSE_MAX + 1> _mouseReleased{};

        double _mouseX = 0.0;
        double _mouseY = 0.0;
        double _mouseDX = 0.0;     
        double _mouseDY = 0.0;     
        double _mouseDXAccum = 0.0; 
        double _mouseDYAccum = 0.0;
        bool   _mouseInited = false;

        double _scrollX = 0.0;     
        double _scrollY = 0.0;
        double _scrollXAccum = 0.0;
        double _scrollYAccum = 0.0;

        int _width = 0;
        int _height = 0;

        std::vector<KeyFn>        _onKey;
        std::vector<CharFn>       _onChar;
        std::vector<MouseBtnFn>   _onMouseButton;
        std::vector<MouseMoveFn>  _onMouseMove;
        std::vector<ScrollFn>     _onScroll;
        std::vector<ResizeFn>     _onResize;
    };
}