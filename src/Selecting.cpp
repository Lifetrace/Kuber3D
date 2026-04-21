#include "Buffers.hpp"
#include "Selecting.hpp"

#include "VectorMath.hpp"

#include "PointActions.hpp"

#include <string>

void Engine::Selecting::Select(int index)
{
    if (index < 0 || index >= (int)Engine::Buffers::positions.size())
        return;
    if (selectedPointsByColor.contains(index))
        return;

    selectedPointsByColor.emplace(index, Engine::Buffers::colors[index]);
    selectedPointsByCoords.emplace(index, Engine::Buffers::positions[index]);
    selectedOrder.push_back(index);
}

void Engine::Selecting::DeSelect(int index)
{
    if (index < 0 || index >= (int)Engine::Buffers::positions.size())
        return;

    auto it = selectedPointsByColor.find(index);
    if (it != selectedPointsByColor.end())
    {
        const glm::vec4 &c = it->second;
        Engine::Buffers::ChangeColor(index, c.r, c.g, c.b, c.a);
        selectedPointsByColor.erase(it);
    }

    selectedPointsByCoords.erase(index);

    selectedOrder.erase(
        std::remove(selectedOrder.begin(), selectedOrder.end(), index),
        selectedOrder.end());
}

void Engine::Selecting::DesAllSelected()
{
    for (auto &[index, c] : selectedPointsByColor)
    {
        Engine::Buffers::ChangeColor(index, c.r, c.g, c.b, c.a);
    }

    selectedPointsByCoords.clear();
    selectedPointsByColor.clear();
    selectedOrder.clear();
}

void Engine::Selecting::DelAllSelected()
{
    std::vector<int> selectedIdx = selectedOrder;
    Engine::VectorMath::SortUnique(selectedIdx);

    if (selectedIdx.empty())
        return;

    const int n = (int)Engine::Buffers::positions.size();

    std::vector<char> removed(n, 0);
    for (int idx : selectedIdx)
    {
        if (idx >= 0 && idx < n)
            removed[idx] = 1;
    }

    std::vector<int> mapOldToNew(n, -1);
    int newN = 0;
    for (int i = 0; i < n; i++)
    {
        if (!removed[i])
            mapOldToNew[i] = newN++;
    }

    std::vector<glm::vec3> newPos;
    newPos.reserve(newN);
    std::vector<glm::vec4> newCol;
    newCol.reserve(newN);

    for (int i = 0; i < n; i++)
    {
        if (!removed[i])
        {
            newPos.push_back(Engine::Buffers::positions[i]);
            newCol.push_back(Engine::Buffers::colors[i]);
        }
    }

    Engine::Buffers::positions = std::move(newPos);
    Engine::Buffers::colors = std::move(newCol);

    std::vector<GLuint> newLines;
    newLines.reserve(Engine::Buffers::lineIndices.size());

    for (size_t i = 0; i + 1 < Engine::Buffers::lineIndices.size(); i += 2)
    {
        int a = (int)Engine::Buffers::lineIndices[i];
        int b = (int)Engine::Buffers::lineIndices[i + 1];

        if (a < 0 || b < 0 || a >= n || b >= n)
            continue;
        if (removed[a] || removed[b])
            continue;

        newLines.push_back((GLuint)mapOldToNew[a]);
        newLines.push_back((GLuint)mapOldToNew[b]);
    }
    std::vector<std::string> newNames;
    newNames.reserve(newN);

    for (int i = 0; i < n; i++)
    {
        if (!removed[i])
        {
            if (i < (int)Engine::PointActions::pointNames.size())
                newNames.push_back(Engine::PointActions::pointNames[i]);
            else
                newNames.push_back("");
        }
    }

    Engine::PointActions::pointNames = std::move(newNames);
    Engine::Buffers::lineIndices = std::move(newLines);

    if ((int)Engine::Buffers::connectedPoints.size() == n)
    {
        std::vector<std::vector<GLuint>> newAdj;
        newAdj.resize(newN);

        for (int i = 0; i < n; i++)
        {
            if (removed[i])
                continue;
            const int ni = mapOldToNew[i];

            for (GLuint nb : Engine::Buffers::connectedPoints[i])
            {
                int j = (int)nb;
                if (j < 0 || j >= n)
                    continue;
                if (removed[j])
                    continue;

                const GLuint nj = (GLuint)mapOldToNew[j];

                auto &row = newAdj[ni];
                if (std::find(row.begin(), row.end(), nj) == row.end())
                    row.push_back(nj);
            }
        }

        Engine::Buffers::connectedPoints = std::move(newAdj);
    }

    selectedOrder.clear();
    selectedPointsByColor.clear();
    selectedPointsByCoords.clear();

    Engine::Buffers::Update();
}

void Engine::Selecting::ToggleSelect(int index)
{
    if (selectedPointsByColor.contains(index))
        DeSelect(index);
    else
        Select(index);
}

void Engine::Selecting::ResetSelection()
{
    selectedPointsByColor.clear();
    selectedPointsByCoords.clear();
    selectedOrder.clear();
}