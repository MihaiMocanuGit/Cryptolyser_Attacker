#pragma once

#include "../WindowI/WindowI.hpp"
#include "App/JobFilter/JobFilter.hpp"
#include "App/Workable/Workable.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

namespace GUI
{
class WindowFilter : public WindowI, public App::Workable
{
  private:
    App::BuffersFilter m_buffers {};

  public:
    explicit WindowFilter(App::WorkloadManager &workloadManager);

    [[nodiscard]] std::unique_ptr<App::JobI> job() const override;
    ;

    void constructWindow() override;
};
} // namespace GUI
