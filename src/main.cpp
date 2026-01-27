#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Engine.hpp"
#include "Events.hpp"
#include "Shader.hpp"
#include "TetrahedronWire.hpp"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>
#include <chrono>

using Clock =  std::chrono::steady_clock;

const float color[4] = {0.15f, 0.15f, 0.15f, 1.0f};
int width = 1280;
int height = 720;

int main(){
    Engine::Window::Init(width, height, "Window", color);
    glfwSwapInterval(1);

    Engine::Events::Get().Init(Engine::Window::window);
    auto& event = Engine::Events::Get();

    Engine::Shader shader("assets/shaders/basic.vert", "assets/shaders/basic.frag");
    Engine::TetrahedronWire tet;
    tet.Init(0.6f);


    float s = 1.0f;

    glEnable(GL_DEPTH_TEST);
    float angleX = 0.0f;
    float angleY = 0.0f;

    auto lastTime = Clock::now();
    int frames = 0;

    while (!Engine::Window::isShouldClose(Engine::Window::window))
    {
        Engine::Window::ClearScreen(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Engine::Events::PollEvents();
        Engine::Events::Get().Update();

        shader.use();
        shader.setFloat("uTime", Engine::Window::GetTime());

        if (Engine::Events::Get().IsKeyPressed(GLFW_KEY_ESCAPE)){
            glfwSetWindowShouldClose(Engine::Window::window, true);
        }

        glm::mat4 view = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 3.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f) 
        );

        glm::mat4 model(1.0f);

        if (Engine::Events::Get().IsKeyDown(GLFW_KEY_RIGHT)) angleX += 0.03f;
        if (Engine::Events::Get().IsKeyDown(GLFW_KEY_LEFT))  angleX -= 0.03f;
        if (Engine::Events::Get().IsKeyDown(GLFW_KEY_UP)) angleY += 0.03f;
        if (Engine::Events::Get().IsKeyDown(GLFW_KEY_DOWN))  angleY -= 0.03f;
        if (Engine::Events::Get().IsKeyPressed(GLFW_KEY_R)){  angleX = 0.0f; angleY = 0.0f;  }
        if (Engine::Events::Get().IsKeyDown(GLFW_KEY_H))  s += 0.05f;
        if (Engine::Events::Get().IsKeyDown(GLFW_KEY_G) && s >= 0.1f)  s -= 0.05f;

        auto& e = Engine::Events::Get();

        if (e.IsMousePressed(GLFW_MOUSE_BUTTON_RIGHT))
            glfwSetInputMode(Engine::Window::window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        if (e.IsMouseReleased(GLFW_MOUSE_BUTTON_RIGHT))
            glfwSetInputMode(Engine::Window::window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        if (e.IsMouseDown(GLFW_MOUSE_BUTTON_RIGHT)) {
            angleX += (float)e.MouseDX() * 0.003f;
            angleY -= (float)e.MouseDY() * 0.003f;
        }

        model = glm::scale(model, glm::vec3(s, s, s));

        model = glm::rotate(model, angleX, glm::vec3(0,1,0));

        model = glm::rotate(model, angleY, glm::vec3(1,0,0));
        
        float aspect = (float)width / (float)height;
        
        glm::mat4 proj = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 100.0f);
        glm::mat4 mvp = proj * view * model;
        
        tet.Draw(shader, mvp);

        frames++;
        auto now = Clock::now();
        std::chrono::duration<double> elapsed = now - lastTime;

        if (elapsed.count() >= 1.0) {
            double fps = frames / elapsed.count();
            std::cout << "FPS: " << fps << "\n";
            frames = 0;
            lastTime = now;
        }

        Engine::Window::SwapBuffers(Engine::Window::window);
    }
    tet.Destroy();
    Engine::Window::Terminate();
    return 0;
}