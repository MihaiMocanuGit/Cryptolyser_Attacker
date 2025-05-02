#include "WindowFilter.hpp"

#include "../Helpers.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

GUI::WindowFilter::WindowFilter(std::string_view name, App::NewWorkloadManager &workloadManager)
    : WindowI {name}, App::Workable {workloadManager}
{
    m_buffers.savePath = std::filesystem::current_path().string();
    m_buffers.loadPath = std::filesystem::current_path().string();
}

std::unique_ptr<App::JobI> GUI::WindowFilter::job() const
{
    return std::make_unique<App::JobFilter>(m_buffers, m_workloadManager.continueRunning());
}

void GUI::WindowFilter::constructWindow()
{
    ImGui::Begin(m_name.c_str());
    ImGui::Text("This is where you can filter the raw timing data.");
    ImGui::SetNextItemWidth(65.0f);
    if (ImGui::InputFloat("Lower Bound", &m_buffers.lb))
    {
        m_buffers.lb = std::clamp(m_buffers.lb, 0.0f, std::numeric_limits<float>::max());
    }

    ImGui::SetNextItemWidth(65.0f);
    if (ImGui::InputFloat("Upper Bound", &m_buffers.ub))
    {
        m_buffers.ub = std::clamp(m_buffers.ub, 0.0f, std::numeric_limits<float>::max());
    }
    ImGui::SetNextItemWidth(Helpers::pathTextFieldWidth);
    if (ImGui::InputText("Load Path", &m_buffers.loadPath, ImGuiInputTextFlags_ElideLeft))
    {
    }
    ImGui::SetNextItemWidth(Helpers::pathTextFieldWidth);
    if (ImGui::InputText("Save Path", &m_buffers.savePath, ImGuiInputTextFlags_ElideLeft))
    {
    }
    if (ImGui::Button("Queue Filter"))
    {
        m_workloadManager.addJob(job());
    }
    ImGui::End();
}
