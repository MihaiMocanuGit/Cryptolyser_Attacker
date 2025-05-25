#include "WindowStudy.hpp"

#include "../Widgets/Widgets.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

GUI::WindowStudy::WindowStudy(std::string_view name, App::WorkloadManager &workloadManager)
    : WindowI {name}, App::Workable {workloadManager}
{
    m_buffers.savePath = std::filesystem::current_path().string();
}

std::unique_ptr<App::JobI> GUI::WindowStudy::job() const
{
    return std::make_unique<App::JobStudy>(m_buffers, m_workloadManager.continueRunning());
}

void GUI::WindowStudy::constructWindow()
{
    ImGui::Begin(m_name.c_str());

    ImGui::Text("This is where you can start a study session.");
    ImGui::SetNextItemWidth(30.0f);
    if (ImGui::InputScalar("##ip0", ImGuiDataType_U8, &m_buffers.ip[0]))
    {
    }
    ImGui::SameLine();
    ImGui::Text(".");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(30.0f);
    if (ImGui::InputScalar("##ip1", ImGuiDataType_U8, &m_buffers.ip[1]))
    {
    }
    ImGui::SameLine();
    ImGui::Text(".");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(30.0f);
    if (ImGui::InputScalar("##ip2", ImGuiDataType_U8, &m_buffers.ip[2]))
    {
    }
    ImGui::SameLine();
    ImGui::Text(".");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(30.0f);
    if (ImGui::InputScalar("##ip3", ImGuiDataType_U8, &m_buffers.ip[3]))
    {
    }
    ImGui::SameLine();
    ImGui::Text(":");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(45.0f);
    if (ImGui::InputScalar("IP : PORT", ImGuiDataType_U16, &m_buffers.port))
    {
    }
    Widgets::fileExplorerWidget(m_buffers.savePath, "Save Path", "Search##1");
    ImGui::SetNextItemWidth(180.0f);
    if (ImGui::InputScalar("Packet count", ImGuiDataType_U64, &m_buffers.packetCount))
    {
    }
    if (ImGui::Checkbox("Automatic Calibration", &m_buffers.calibrate))
    {
    }
    if (m_buffers.calibrate)
    {
        ImGui::SetNextItemWidth(65.0f);
        if (ImGui::InputFloat("Lower Bound confidence", &m_buffers.lbConfidence, 0.0f, 0.0f,
                              "%.5f"))
        {
            m_buffers.lbConfidence = std::clamp(m_buffers.lbConfidence, 0.0f, 1.0f);
        }

        ImGui::SetNextItemWidth(65.0f);
        if (ImGui::InputFloat("Upper Bound confidence", &m_buffers.ubConfidence, 0.0f, 0.0f,
                              "%.5f"))
        {
            m_buffers.ubConfidence = std::clamp(m_buffers.ubConfidence, 0.0f, 1.0f);
        }
    }
    else
    {
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
    }
    if (ImGui::Checkbox("Known Key", &m_buffers.knownKey))
    {
    }
    if (m_buffers.knownKey)
    {
        for (unsigned i {0}; i < PACKET_KEY_BYTE_SIZE; ++i)
        {
            ImGui::SetNextItemWidth(30.0f);
            std::string id {(i != PACKET_KEY_BYTE_SIZE - 1) ? "##key" + std::to_string(i) : "Key"};
            if (ImGui::InputScalar(id.c_str(), ImGuiDataType_U8, &m_buffers.key[i], NULL, NULL,
                                   "%02x"))
            {
            }
            if (i != PACKET_KEY_BYTE_SIZE - 1)
                ImGui::SameLine();
        }
    }
    if (ImGui::Button("Queue Study Run"))
        m_workloadManager.addJob(job());

    ImGui::End();
}
