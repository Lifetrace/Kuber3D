#pragma once

#include <string>
#include <cstdint>
#include <variant>
#include "GLincs.hpp"
#include <glm/vec2.hpp>

typedef unsigned int uint;

namespace Engine
{
    class Events
    {
    public:
        static bool *_keys;
        static uint *_frames;
        static uint _current;
        static float deltaX;
        static float deltaY;
        static float x;
        static float y;
        static bool _cursor_locked;
        static bool _cursor_started;

        static int initialize();
        static void PollEvents();

        static bool Pressed(int keycode);
        static bool jPressed(int keycode);

        static bool clicked(int button);
        static bool jclicked(int button);

        static inline glm::vec2 MouseToNDC(double mouseX, double mouseY, int winW, int winH)
        {
            float x = (2.0f * (float)mouseX) / (float)winW - 1.0f;
            float y = 1.0f - (2.0f * (float)mouseY) / (float)winH;
            return {x, y};
        }
    };
}