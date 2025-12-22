#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>


GLfloat points[] = {
//       x      y      z       r      g      b
/*1*/  -0.5f, -0.5f,  0.0f,   1.0f,  1.0f,  1.0f,
/*2*/   0.5f, -0.5f,  0.0f,   1.0f,  1.0f,  1.0f,
/*3*/   0.0f,  0.5f,  0.0f,   1.0f,  1.0f,  1.0f,
};

GLuint indices[] = {
  0, 1,
  1, 2,
  2, 0
};

const char* vertexShaderSource = R"(
#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
out vec3 ourColor;
void main()
{
    gl_Position = vec4(aPos, 1.0);
    ourColor = aColor;
    gl_PointSize = 8.0;
}
)";

const char* fragmentShaderSource = R"(
#version 460 core
in vec3 ourColor;
out vec4 FragColor;

uniform int uIsPoint; 

void main()
{
    float alpha = 1.0;

    if (uIsPoint == 1)
    {
        vec2 uv = gl_PointCoord * 2.0 - 1.0;
        float r = length(uv);

        float radius = 1.0;
        float aa = fwidth(r);

        alpha = 1.0 - smoothstep(radius - aa, radius + aa, r);
        if (alpha <= 0.0) discard;
    }

    FragColor = vec4(ourColor, alpha);
}
)";

int main(){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Kuber3D", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    GLuint v, f;
    v = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(v, 1, &vertexShaderSource, NULL);
    glCompileShader(v);

    f = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(f, 1, &fragmentShaderSource, NULL);
    glCompileShader(f);

    GLuint shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, v);
    glAttachShader(shaderProgram, f);

    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);
    int uIsPointLoc = glGetUniformLocation(shaderProgram, "uIsPoint");

    GLuint VAO, VBO;
    glCreateVertexArrays(1, &VAO);
    glCreateBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), (void*)(3*sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    GLuint EBO;
    glCreateBuffers(1, &EBO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glLineWidth(2.0f);

    glBindVertexArray(0);

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, 800, 600);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);


    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(VAO);

        glUniform1i(uIsPointLoc, 1);
        glDrawArrays(GL_POINTS, 0, 3);

        glUniform1i(uIsPointLoc, 0);
        glDrawElements(GL_LINES, 6, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
