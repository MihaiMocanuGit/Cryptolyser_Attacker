#include "WindowFilter.hpp"

GUI::WindowFilter::WindowFilter(App::WorkloadManager &workloadManager)
    : App::Workable {workloadManager}
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
    ImGui::Begin("Filter");
    ImGui::Text("This is where you can filter the raw timing data.");
    ImGui::SetNextItemWidth(65.0f);
    if (ImGui::InputFloat("Lower Bound", &m_buffers.lb))
    {
        m_buffers.lb = std::clamp(m_buffers.lb, 0.0f, std::numeric_limits<float>::max());
        std::cout << "Filter buffer lower bound: " << m_buffers.lb << std::endl;
    }

    ImGui::SetNextItemWidth(65.0f);
    if (ImGui::InputFloat("Upper Bound", &m_buffers.ub))
    {
        m_buffers.ub = std::clamp(m_buffers.ub, 0.0f, std::numeric_limits<float>::max());
        std::cout << "Filter buffer upper bound: " << m_buffers.ub << std::endl;
    }
    ImGui::SetNextItemWidth(250.0f);
    if (ImGui::InputText("Load Path", &m_buffers.loadPath, ImGuiInputTextFlags_ElideLeft))
    {
        std::cout << "Filter buffer load path: " << m_buffers.loadPath << std::endl;
    }
    ImGui::SetNextItemWidth(250.0f);
    if (ImGui::InputText("Save Path", &m_buffers.savePath, ImGuiInputTextFlags_ElideLeft))
    {
        std::cout << "Filter buffer save path: " << m_buffers.savePath << std::endl;
    }
    if (ImGui::Button("Queue Filter JobCombineData"))
    {
        std::cout << "Queue Filter button pressed" << std::endl;

        m_workloadManager.push(job());
    }
    ImGui::End();
}
