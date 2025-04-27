#pragma once

#include "../WindowI/WindowI.hpp"
#include "App/JobDataCombine/JobCombineData.hpp"
#include "App/Workable/Workable.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

namespace GUI
{
class WindowCombineData : public WindowI, public App::Workable
{
  private:
    App::BuffersDataCombine m_buffers {};

  public:
    explicit WindowCombineData(App::WorkloadManager &workloadManager);

    [[nodiscard]] std::unique_ptr<App::JobI> job() const override;
    ;

    void constructWindow() override;
};
} // namespace GUI
