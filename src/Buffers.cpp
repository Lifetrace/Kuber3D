#include "Buffers.hpp"
#include <iostream>
#include <algorithm>
#include <vector>

void Engine::Buffers::Init(){
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &eboLines);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboLines);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    
    glGenBuffers(1, &vboPos);

    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glGenBuffers(1, &vboCol);

    glBindBuffer(GL_ARRAY_BUFFER, vboCol);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glGenBuffers(1, &eboFaces);

    glBindVertexArray(vao);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboFaces);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faceIndices.size() * sizeof(unsigned int), faceIndices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void Engine::Buffers::AddPoint(float x, float y, float z, float r, float g, float b, float a){
    positions.push_back(glm::vec3(x, y, z));
    colors.push_back(glm::vec4(r, g, b, a));

    connectedPoints.emplace_back();

    Update();
}

void Engine::Buffers::DeletePoint(int index)
{
    const int n = (int)positions.size();
    if (index < 0 || index >= n) return;

    positions.erase(positions.begin() + index);
    colors.erase(colors.begin() + index);

    std::vector<GLuint> newLineIndices;
    newLineIndices.reserve(lineIndices.size());

    for (size_t i = 0; i + 1 < lineIndices.size(); i += 2)
    {
        int a = (int)lineIndices[i];
        int b = (int)lineIndices[i + 1];

        if (a == index || b == index)
            continue;

        if (a > index) --a;
        if (b > index) --b;

        if (a < 0 || b < 0 || a == b) continue;
        if (a >= (int)positions.size() || b >= (int)positions.size()) continue;

        newLineIndices.push_back((GLuint)a);
        newLineIndices.push_back((GLuint)b);
    }

    lineIndices.swap(newLineIndices);

    std::vector<GLuint> newFaceIndices;
    newFaceIndices.reserve(faceIndices.size());

    for (size_t i = 0; i + 2 < faceIndices.size(); i += 3)
    {
        int a = (int)faceIndices[i];
        int b = (int)faceIndices[i + 1];
        int c = (int)faceIndices[i + 2];

        if (a == index || b == index || c == index)
            continue;

        if (a > index) --a;
        if (b > index) --b;
        if (c > index) --c;

        if (a < 0 || b < 0 || c < 0) continue;
        if (a >= (int)positions.size() || b >= (int)positions.size() || c >= (int)positions.size()) continue;
        if (a == b || b == c || a == c) continue;

        newFaceIndices.push_back((GLuint)a);
        newFaceIndices.push_back((GLuint)b);
        newFaceIndices.push_back((GLuint)c);
    }

    faceIndices.swap(newFaceIndices);

    connectedPoints.clear();
    connectedPoints.resize(positions.size());

    for (size_t i = 0; i + 1 < lineIndices.size(); i += 2)
    {
        const GLuint a = lineIndices[i];
        const GLuint b = lineIndices[i + 1];

        if (a >= connectedPoints.size() || b >= connectedPoints.size()) continue;
        if (a == b) continue;

        if (!HasNeighbor(connectedPoints[a], b)) connectedPoints[a].push_back(b);
        if (!HasNeighbor(connectedPoints[b], a)) connectedPoints[b].push_back(a);
    }

    Update();
}

void Engine::Buffers::DeleteAll(){
    positions.clear();
    colors.clear();
    lineIndices.clear();
    faceIndices.clear();
    connectedPoints.clear();
    Update();
}

void Engine::Buffers::ChangeColor(int index, float r, float g, float b, float a){
    colors.at(index) = glm::vec4(r, g, b, a);
    Update();
}

void Engine::Buffers::ConnectPointsLine(int first, int second)
{
    if (first < 0 || second < 0) return;
    if (first == second) return;

    const int n = (int)positions.size();
    if (first >= n || second >= n) return;

    if ((int)connectedPoints.size() != n)
        connectedPoints.resize(n);

    const GLuint a = (GLuint)first;
    const GLuint b = (GLuint)second;

    if (!HasNeighbor(connectedPoints[a], b)) connectedPoints[a].push_back(b);
    if (!HasNeighbor(connectedPoints[b], a)) connectedPoints[b].push_back(a);

    lineIndices.push_back(a);
    lineIndices.push_back(b);

    Update();
}

void Engine::Buffers::DisConnectPointsLine(int first, int second){
    if (first < 0 || second < 0) return;
    if (first == second) return;

    const int n = (int)positions.size();
    if (first >= n || second >= n) return;

    const GLuint a = (GLuint)first;
    const GLuint b = (GLuint)second;

    if ((int)connectedPoints.size() == n){
        RemoveNeighbor(connectedPoints[a], b);
        RemoveNeighbor(connectedPoints[b], a);
    }

    for (size_t i = 0; i + 1 < lineIndices.size(); i += 2)
    {
        int x = (int)lineIndices[i];
        int y = (int)lineIndices[i + 1];

        if ((x == first && y == second) || (x == second && y == first))
        {
            lineIndices.erase(lineIndices.begin() + (long)i, lineIndices.begin() + (long)i + 2);
            break;
        }
    }

    Update();
}


void Engine::Buffers::Update(){
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glBufferData(GL_ARRAY_BUFFER,
        (GLsizeiptr)(positions.size() * sizeof(glm::vec3)),
        positions.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vboCol);
    glBufferData(GL_ARRAY_BUFFER,
        (GLsizeiptr)(colors.size() * sizeof(glm::vec4)),
        colors.data(), GL_DYNAMIC_DRAW);

    // линии
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboLines);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        (GLsizeiptr)(lineIndices.size() * sizeof(GLuint)),
        lineIndices.data(), GL_DYNAMIC_DRAW);

    // грани
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboFaces);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        (GLsizeiptr)(faceIndices.size() * sizeof(GLuint)),
        faceIndices.data(), GL_DYNAMIC_DRAW);

    glBindVertexArray(0);

    if (positions.size() != colors.size())
        throw std::runtime_error("positions/colors size mismatch");
}


void Engine::Buffers::DrawLines(){
    glBindVertexArray(vao);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboLines);

    if (lineIndices.size() >= 2)
    {
        glDrawElements(GL_LINES, (GLsizei)lineIndices.size(), GL_UNSIGNED_INT, (void*)0);
    }

    glBindVertexArray(0);
}


void Engine::Buffers::DrawPoints(){
    glBindVertexArray(vao);

    glDrawArrays(GL_POINTS, 0, (GLsizei)positions.size());

    glBindVertexArray(0);
}

void Engine::Buffers::DrawFaces(){
    glBindVertexArray(vao);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboFaces);

    if (faceIndices.size() >= 3)
    {
        glDrawElements(GL_TRIANGLES, (GLsizei)faceIndices.size(), GL_UNSIGNED_INT, (void*)0);
    }

    glBindVertexArray(0);
}

void Engine::Buffers::AddFace(unsigned int a, unsigned int b, unsigned int c){
    faceIndices.push_back(a);
    faceIndices.push_back(b);
    faceIndices.push_back(c);
}

void Engine::Buffers::AddQuad(unsigned int a, unsigned int b, unsigned int c, unsigned int d){
    AddFace(a, b, c);
    AddFace(a, c, d);
}


void Engine::Buffers::Destroy(){
    if (eboLines) glDeleteBuffers(1, &eboLines);
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vboPos) glDeleteBuffers(1, &vboPos);
    if (vboCol) glDeleteBuffers(1, &vboCol);
    if(eboFaces) glDeleteBuffers(1, &eboFaces);

    eboLines = eboFaces = vboPos = vboCol = vao = 0;
    
    lineIndices.clear();
    positions.clear();
    faceIndices.clear();
    colors.clear();
}

