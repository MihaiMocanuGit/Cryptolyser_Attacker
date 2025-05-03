#include "Widgets.hpp"

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "nfd.hpp"

#include <atomic>
#include <iostream>
#include <string>
#include <string_view>
#include <thread>

static constexpr float pathTextFieldWidth {400.0};

void GUI::Widgets::fileExplorerWidget(std::string &r_path, std::string_view textLabel,
                                      std::string_view buttonLabel)
{
    if (r_path.empty())
        r_path = fileExplorerWidget_defaultPath;
    if (ImGui::Button(buttonLabel.data()))
    {
        static std::atomic_bool isExplorerOpen;
        auto explorer = [&r_path]()
        {
            if (isExplorerOpen.load())
                return;
            isExplorerOpen.store(true);
            try
            {
                // TODO: Resolve Memory leak in NFD lib...
                NFD::Guard nfdGuard {};
                NFD::UniquePath outPath;

                nfdresult_t result = NFD::PickFolder(outPath, r_path.c_str());
                if (result == NFD_OKAY)
                {
                    r_path = outPath.get();
                }
                else if (result == NFD_ERROR)
                {
                    std::cerr << "File Explorer error: " << NFD::GetError() << std::endl;
                }
                isExplorerOpen.store(false);
                return;
            }
            catch (...)
            {
                isExplorerOpen.store(false);
                return;
            }
        };
        if (not isExplorerOpen.load())
        {
            std::thread explorerThread {explorer};
            explorerThread.detach();
        }
        else
        {
            std::cout << "Please close the previous explorer window." << std::endl;
        }
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(pathTextFieldWidth);
    if (ImGui::InputText(textLabel.data(), &r_path, ImGuiInputTextFlags_ElideLeft))
    {
    }
}
