#include "WindowCorrelate.hpp"

#include "../Widgets/Widgets.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

GUI::WindowCorrelate::WindowCorrelate(std::string_view name, App::WorkloadManager &workloadManager)
    : WindowI {name}, WorkableWindow {workloadManager}
{
    std::string currentPath {Widgets::fileExplorerWidget_defaultPath};
    m_buffers.groups.emplace_back();
    m_buffers.groups[0].victimLoadPaths.push_back(currentPath);
    m_buffers.groups[0].doppelLoadPaths.push_back(currentPath);
    m_buffers.savePath = currentPath;
    m_buffers.victimKeyKnown = true;
}

std::unique_ptr<App::JobI> GUI::WindowCorrelate::job() const
{
    return std::make_unique<App::JobCorrelate>(m_buffers, m_workloadManager.continueRunning());
}

void GUI::WindowCorrelate::constructWindow()
{
    std::string currentPath {Widgets::fileExplorerWidget_defaultPath};

    ImGui::Begin(m_name.c_str());
    ImGui::Text("This is where you can correlate the studied keys.");
    Widgets::fileExplorerWidget(m_buffers.savePath, "Save Path", "Search##SavePath");
    ImGui::Text("Paths for victim timing data.");
    if (ImGui::Checkbox("Known Victim Key", &m_buffers.victimKeyKnown))
    {
    }
    if (m_buffers.victimKeyKnown)
    {
        for (unsigned i {0}; i < PACKET_KEY_SIZE; ++i)
        {
            ImGui::SetNextItemWidth(30.0f);
            std::string id {(i != PACKET_KEY_SIZE - 1) ? "##key" + std::to_string(i) : "Key"};
            if (ImGui::InputScalar(id.c_str(), ImGuiDataType_U8, &m_buffers.victimKey[i], NULL,
                                   NULL, "%02x"))
            {
            }
            if (i != PACKET_KEY_SIZE - 1)
                ImGui::SameLine();
        }
    }

    auto processGroup = [&](auto &group, unsigned groupId)
    {
        ImGui::BeginChild(std::format("Group{}", groupId).c_str(), ImVec2(0, 0),
                          ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
        ImGui::Text("Group %d:", groupId);
        { // VICTIM PATHS
            if (ImGui::Button("+##Victim"))
            {
                group.victimLoadPaths.push_back(currentPath);
            }
            ImGui::SameLine();
            if (ImGui::Button("-##Victim") and group.victimLoadPaths.size() > 1)
            {
                group.victimLoadPaths.pop_back();
            }
            unsigned index {0};
            for (std::string &inputLine : group.victimLoadPaths)
            {
                Widgets::fileExplorerWidget(inputLine, std::format("Victim Load Path {}", index),
                                            std::format("Search##VictimLoadPath{}", index));
                ++index;
            }
        }
        { // DOPPELGANGER PATHS
            ImGui::Text("Paths for doppel timing data.");
            if (ImGui::Button("+##Doppel"))
            {
                group.doppelLoadPaths.push_back(currentPath);
            }
            ImGui::SameLine();
            if (ImGui::Button("-##Doppel") and group.doppelLoadPaths.size() > 1)
            {
                group.doppelLoadPaths.pop_back();
            }
            unsigned index {0};
            for (std::string &inputLine : group.doppelLoadPaths)
            {
                Widgets::fileExplorerWidget(inputLine, std::format("Doppel Load Path {}", index),
                                            std::format("Search##DoppelLoadPath{}", index));
                ++index;
            }
        }
        ImGui::EndChild();
    };
    ImGui::Text("Correlate groups.");
    if (ImGui::Button("+##Group"))
        m_buffers.groups.push_back(m_buffers.groups.back());
    ImGui::SameLine();
    if (ImGui::Button("-##Group") and m_buffers.groups.size() > 1)
        m_buffers.groups.pop_back();
    for (unsigned groupId {0}; groupId < m_buffers.groups.size(); ++groupId)
        processGroup(m_buffers.groups[groupId], groupId);

    if (ImGui::Button("Queue Correlate job"))
    {
        m_workloadManager.addJob(job());
    }
    ImGui::End();
}
