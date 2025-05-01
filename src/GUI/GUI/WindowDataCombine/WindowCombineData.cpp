#include "WindowCombineData.hpp"

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

GUI::WindowCombineData::WindowCombineData(std::string_view name,
                                          App::WorkloadManager &workloadManager)
    : WindowI {name}, App::Workable {workloadManager}
{
    std::string currentPath {std::filesystem::current_path().string()};
    m_buffers.loadPaths.push_back(currentPath);
    m_buffers.loadPaths.push_back(currentPath);
    m_buffers.savePath = currentPath;
    m_buffers.onlyMetrics = false;
}

std::unique_ptr<App::JobI> GUI::WindowCombineData::job() const
{
    return std::make_unique<App::JobCombineData>(m_buffers, m_workloadManager.continueRunning());
}

void GUI::WindowCombineData::constructWindow()
{
    std::string currentPath {std::filesystem::current_path().string()};

    ImGui::Begin(m_name.c_str());
    ImGui::Text("This is where you can combine two or more raw timing data into a new one.");
    ImGui::SetNextItemWidth(250.0f);
    if (ImGui::InputText("Save Path", &m_buffers.savePath))
    {
    }
    if (ImGui::Button("+"))
    {
        m_buffers.loadPaths.push_back(currentPath);
    }
    ImGui::SameLine();
    if (ImGui::Button("-") and m_buffers.loadPaths.size() > 2)
    {
        m_buffers.loadPaths.pop_back();
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("Combine Only Metrics", &m_buffers.onlyMetrics))
    {
    }
    unsigned index {0};
    for (std::string &inputLine : m_buffers.loadPaths)
    {
        ImGui::SetNextItemWidth(250.0f);
        if (ImGui::InputText(std::format("Load Path {}", index).c_str(), &inputLine))
        {
        }
        ++index;
    }
    if (ImGui::Button("Queue Combine Data job"))
    {
        m_workloadManager.push(job());
    }
    ImGui::End();
}
