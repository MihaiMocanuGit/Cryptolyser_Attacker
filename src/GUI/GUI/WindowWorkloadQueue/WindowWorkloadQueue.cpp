#include "WindowWorkloadQueue.hpp"

#include "GUI/WindowI/WindowI.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

#include <sstream>

namespace GUI
{
std::ostringstream WindowWorkloadQueue::m_coutBuff;

std::ostringstream WindowWorkloadQueue::m_cerrBuff;

void WindowWorkloadQueue::coutInit() { std::cout.rdbuf(m_coutBuff.rdbuf()); };
void WindowWorkloadQueue::cerrInit() { std::cerr.rdbuf(m_cerrBuff.rdbuf()); };

WindowWorkloadQueue::WindowWorkloadQueue(std::string_view name,
                                         App::WorkloadManager &workloadManager)
    : WindowI {name}, m_workloadManager {workloadManager}
{
    coutInit();
    cerrInit();
}

void WindowWorkloadQueue::constructWindow()
{
    ImGui::Begin(m_name.c_str());

    ImGui::Text("This is where you can manage the work queue.");
    constexpr float topWidthRatio {1.0f / 3.0};
    ImGui::BeginChild("##WorkloadControl",
                      {ImGui::GetWindowSize().x / 2, ImGui::GetWindowSize().y * topWidthRatio},
                      ImGuiChildFlags_Borders);
    if (not m_workloadManager.busy())
    {
        ImGui::Text("Workload currently contains %zu new jobs.",
                    m_workloadManager.totalJobsCount() - m_workloadManager.processedJobsCount());
        if (ImGui::Button("Start Workload!"))
            m_workloadManager.start();
    }
    else
    {
        ImGui::Text("Currently running workload... (%zu / %zu)",
                    m_workloadManager.processedJobsCount(), m_workloadManager.totalJobsCount());
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("##WorkloadQueueStart",
                      ImVec2(0.0f, ImGui::GetWindowSize().y * topWidthRatio),
                      ImGuiChildFlags_Border);
    ImGui::Text("Work Queue.");
    ImGui::BeginChild("##WorkloadQueueDescription", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders,
                      ImGuiWindowFlags_AlwaysVerticalScrollbar |
                          ImGuiWindowFlags_AlwaysHorizontalScrollbar);
    const auto &descriptions = m_workloadManager.queuedJobDescriptions();
    const size_t startingQueuePos {m_workloadManager.processedJobsCount()};
    size_t position {1}, offset {0};
    if (m_workloadManager.busy())
        offset++;
    for (const std::string &description : descriptions)
        ImGui::Text("%zu | %s", startingQueuePos + offset + position++, description.c_str());
    ImGui::EndChild();
    ImGui::EndChild();
    ImGui::Text("Console Output.");
    static std::string consoleCombinedOutput = "";

    consoleCombinedOutput += m_cerrBuff.str();
    m_cerrBuff.str("");
    m_cerrBuff.clear();

    consoleCombinedOutput += m_coutBuff.str();
    m_coutBuff.str("");
    m_coutBuff.clear();

    ImGui::BeginChild("##log", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders,
                      ImGuiWindowFlags_AlwaysVerticalScrollbar |
                          ImGuiWindowFlags_AlwaysHorizontalScrollbar);

    ImGui::TextUnformatted(consoleCombinedOutput.c_str());
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);
    ImGui::EndChild();

    ImGui::End();
}
} // namespace GUI
