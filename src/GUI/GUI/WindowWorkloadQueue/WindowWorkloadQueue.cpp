#include "WindowWorkloadQueue.hpp"

#include "../WindowI/WindowI.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

#include <format>
#include <iostream>
#include <sstream>

namespace GUI
{
std::ostringstream WindowWorkloadQueue::m_coutBuff;

std::ostringstream WindowWorkloadQueue::m_cerrBuff;

void WindowWorkloadQueue::coutInit() { std::cout.rdbuf(m_coutBuff.rdbuf()); };
void WindowWorkloadQueue::cerrInit() { std::cerr.rdbuf(m_cerrBuff.rdbuf()); };

WindowWorkloadQueue::WindowWorkloadQueue(std::string_view name, App::JobScheduler &jobScheduler)
    : WindowI {name}, m_jobScheduler {jobScheduler}
{
    coutInit();
    cerrInit();
}

void WindowWorkloadQueue::constructWindow()
{
    ImGui::Begin(m_name.c_str());

    ImGui::TextWrapped("This is where you can manage the work queue.");
    constexpr float topWidthRatio {1.0f / 3.0};
    ImGui::BeginChild("##WorkloadControl",
                      {ImGui::GetWindowSize().x / 3.5f, ImGui::GetWindowSize().y * topWidthRatio},
                      ImGuiChildFlags_Borders);

    size_t noJobs {m_jobScheduler.jobqueueSize()};
    size_t currentJob {m_jobScheduler.currentJobIndex()};

    switch (m_jobScheduler.state())
    {
        case App::JobScheduler::States::NOT_STARTED:
        {
            ImGui::TextWrapped("The Sheduler is not running.");
            ImGui::TextWrapped("Number of jobs: %zu", noJobs);
            if (noJobs > 0)
            {
                if (ImGui::Button("Run queued jobs"))
                    m_jobScheduler.run();
                if (ImGui::Button("Remove all jobs"))
                    m_jobScheduler.removeAllJobs();
            }
            break;
        }
        case App::JobScheduler::States::RUNNING:
        {
            ImGui::TextWrapped("The Manager is currently running.");
            ImGui::TextWrapped("Number of jobs: %zu", noJobs);
            ImGui::TextWrapped("Processed number of jobs: %zu", currentJob);
            if (noJobs > 0)
            {
                if (ImGui::Button("Remove all jobs"))
                    m_jobScheduler.removeAllJobs();
                if (ImGui::Button("Pause after this job."))
                    m_jobScheduler.pauseAfter();
            }
            break;
        }
        case App::JobScheduler::States::PAUSED:
        {
            ImGui::TextWrapped("The Manager paused.");
            ImGui::TextWrapped("Number of jobs: %zu", noJobs);
            ImGui::TextWrapped("Processed number of jobs: %zu", currentJob);
            if (noJobs > 0)
            {
                if (ImGui::Button("Remove all jobs"))
                    m_jobScheduler.removeAllJobs();
                if (ImGui::Button("Resume"))
                    m_jobScheduler.resume();
            }
            break;
        }
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
    const auto &descriptions = m_jobScheduler.allJobDescriptions();
    for (size_t index {0}; index < descriptions.size(); ++index)
    {
        if (ImGui::Button(std::format("-##{}", index).c_str()))
        {
            try
            {
                m_jobScheduler.removeJob(index);
            }
            catch (...)
            {
                std::cout << "Job with index " << index << " cannot be removed." << std::endl;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button(std::format("^##{}", index).c_str()))
        {
            try
            {
                m_jobScheduler.swapJobs(index, index - 1);
            }
            catch (...)
            {
                std::cout << "Job with index " << index << " cannot be moved up." << std::endl;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button(std::format("v##{}", index).c_str()))
        {
            try
            {
                m_jobScheduler.swapJobs(index, index + 1);
            }
            catch (...)
            {
                std::cout << "Job with index " << index << " cannot be moved down." << std::endl;
            }
        }
        ImGui::SameLine();
        if (index != currentJob)
            ImGui::Text("%zu | %s", index, descriptions[index].c_str());
        else
            ImGui::Text("HERE --> %zu | %s", index, descriptions[index].c_str());
    }
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
