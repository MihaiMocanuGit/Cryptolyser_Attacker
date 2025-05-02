#include "WindowCorrelate.hpp"

#include "../Helpers.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

GUI::WindowCorrelate::WindowCorrelate(std::string_view name,
                                      App::NewWorkloadManager &workloadManager)
    : WindowI {name}, App::Workable {workloadManager}
{
    std::string currentPath {Helpers::pathTextDefaultLocation};
    m_buffers.victimLoadPaths.push_back(currentPath);
    m_buffers.doppelLoadPaths.push_back(currentPath);
    m_buffers.savePath = currentPath;
    m_buffers.victimKeyKnown = true;
}

std::unique_ptr<App::JobI> GUI::WindowCorrelate::job() const
{
    return std::make_unique<App::JobCorrelate>(m_buffers, m_workloadManager.continueRunning());
}

void GUI::WindowCorrelate::constructWindow()
{
    std::string currentPath {Helpers::pathTextDefaultLocation};

    ImGui::Begin(m_name.c_str());
    ImGui::Text("This is where you can correlate the studied keys.");
    ImGui::SetNextItemWidth(Helpers::pathTextFieldWidth);
    if (ImGui::InputText("Save Path", &m_buffers.savePath, ImGuiInputTextFlags_ElideLeft))
    {
    }
    {
        ImGui::Text("Paths for victim timing data.");
        if (ImGui::Checkbox("Known Victim Key", &m_buffers.victimKeyKnown))
        {
        }
        if (m_buffers.victimKeyKnown)
        {
            for (unsigned i {0}; i < PACKET_KEY_BYTE_SIZE; ++i)
            {
                ImGui::SetNextItemWidth(30.0f);
                std::string id {(i != PACKET_KEY_BYTE_SIZE - 1) ? "##key" + std::to_string(i)
                                                                : "Key"};
                if (ImGui::InputScalar(id.c_str(), ImGuiDataType_U8, &m_buffers.victimKey[i]))
                {
                }
                if (i != PACKET_KEY_BYTE_SIZE - 1)
                    ImGui::SameLine();
            }
        }
        if (ImGui::Button("+##Victim"))
        {
            m_buffers.victimLoadPaths.push_back(currentPath);
        }
        ImGui::SameLine();
        if (ImGui::Button("-##Victim") and m_buffers.victimLoadPaths.size() > 1)
        {
            m_buffers.victimLoadPaths.pop_back();
        }
        unsigned index {0};
        for (std::string &inputLine : m_buffers.victimLoadPaths)
        {
            ImGui::SetNextItemWidth(Helpers::pathTextFieldWidth);
            if (ImGui::InputText(std::format("Victim Load Path {}", index).c_str(), &inputLine,
                                 ImGuiInputTextFlags_ElideLeft))
            {
            }
            ++index;
        }
    }
    {
        ImGui::Text("Paths for doppel timing data.");
        if (ImGui::Button("+##Doppel"))
        {
            m_buffers.doppelLoadPaths.push_back(currentPath);
        }
        ImGui::SameLine();
        if (ImGui::Button("-##Doppel") and m_buffers.doppelLoadPaths.size() > 1)
        {
            m_buffers.doppelLoadPaths.pop_back();
        }
        unsigned index {0};
        for (std::string &inputLine : m_buffers.doppelLoadPaths)
        {
            ImGui::SetNextItemWidth(Helpers::pathTextFieldWidth);
            if (ImGui::InputText(std::format("Doppel Load Path {}", index).c_str(), &inputLine,
                                 ImGuiInputTextFlags_ElideLeft))
            {
            }
            ++index;
        }
    }

    if (ImGui::Button("Queue Correlate job"))
    {
        m_workloadManager.addJob(job());
    }
    ImGui::End();
}
