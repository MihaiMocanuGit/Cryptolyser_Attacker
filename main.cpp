/// PROJECT INCLUDES
///
#include "App/WorkloadManager/WorkloadManager.hpp"
#include "GUI/GUI.hpp"
#include "GUI/WindowDataCombine/WindowCombineData.hpp"
#include "GUI/WindowFilter/WindowFilter.hpp"
#include "GUI/WindowStudy/WindowStudy.hpp"
#include "GUI/WindowWorkloadQueue/WindowWorkloadQueue.hpp"
/// THIRD PARTY INCLUDES
///
#include "SDL.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl2.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include "SDL_opengles2.h"
#else
#include "SDL_opengl.h"
#endif
/// STL INCLUDES
///
#include <algorithm>
#include <csignal>
#include <iostream>

namespace
{
std::atomic_flag g_continueRunning {true};
void exitHandler(int signal)
{
    static int count = 0;
    (void)signal;
    if (count == 0)
    {
        std::cout
            << "Captured a quit signal, trying to gracefully stop. Send the signal one more time "
               "to forcefully kill the app."
            << std::endl;
    }
    else
    {
        std::cout << "Forced quit." << std::endl;
        exit(EXIT_SUCCESS);
    }
    g_continueRunning.clear();
    count++;
}
} // namespace

int main(int argc, char **argv)
{
    struct sigaction sigactionExit {};
    sigactionExit.sa_handler = exitHandler;
    sigemptyset(&sigactionExit.sa_mask);
    sigactionExit.sa_flags = SA_RESTART;
    for (auto sig : {SIGTERM, SIGINT, SIGQUIT})
    {
        if (sigaction(sig, &sigactionExit, nullptr))
        {
            std::perror("Error setting signal handler\n");
            return EXIT_FAILURE;
        }
    }

    GUI::AppGUI gui {g_continueRunning};
    gui.init();

#ifdef NDEBUG
    bool show_demo_window = false;
#else
    bool show_demo_window = true;
#endif

    App::WorkloadManager workloadManager {g_continueRunning};
    GUI::WindowWorkloadQueue workloadQueueWindow {"Workload", workloadManager};
    GUI::WindowStudy studyWindow {"Study", workloadManager};
    GUI::WindowFilter filterWindow {"Filter", workloadManager};
    GUI::WindowCombineData combineDataWindow {"CombineData", workloadManager};

    gui.runEveryFrame(
        [&]()
        {
            static bool firstFrame {true};

            ImGuiID dockspace {ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport())};
            if (firstFrame)
            {

                ImGuiID id = dockspace;
                ImGui::DockBuilderRemoveNode(id);
                ImGui::DockBuilderAddNode(id);

                ImVec2 size {600, 300};

                ImGui::DockBuilderSetNodeSize(id, size);
                ImGui::DockBuilderSetNodePos(id, {0.0f, 0.0f});

                ImGuiID dock1 =
                    ImGui::DockBuilderSplitNode(id, ImGuiDir_Up, 1 / 3.0f, nullptr, &id);
                ImGuiID dock2 =
                    ImGui::DockBuilderSplitNode(id, ImGuiDir_Down, 2 / 3.0f, nullptr, &id);

                ImGui::DockBuilderDockWindow(studyWindow.name().data(), dock1);
                ImGui::DockBuilderDockWindow(filterWindow.name().data(), dock1);
                ImGui::DockBuilderDockWindow(combineDataWindow.name().data(), dock1);
                ImGui::DockBuilderDockWindow("Correlate", dock1);
                ImGui::DockBuilderDockWindow(workloadQueueWindow.name().data(), dock2);

                ImGui::DockBuilderFinish(id);
            }

            if (show_demo_window)
                ImGui::ShowDemoWindow(&show_demo_window);

            workloadQueueWindow.constructWindow();
            studyWindow.constructWindow();
            filterWindow.constructWindow();
            combineDataWindow.constructWindow();

            {
                ImGui::Begin("Correlate");

                ImGui::Text("This is where you can correlate the studied keys.");

                ImGui::End();
            }

            firstFrame = false;
        });
    return 0;
}
