#pragma once

#include "../WindowI/WindowI.hpp"
#include "App/JobStudy/JobStudy.hpp"
#include "App/Workable/Workable.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

namespace GUI
{
class WindowStudy : public WindowI, public App::Workable
{
  private:
    App::BuffersStudy m_buffers {};

  public:
    explicit WindowStudy(App::WorkloadManager &workloadManager);

    [[nodiscard]] std::unique_ptr<App::JobI> job() const override;

    void constructWindow() override;
};
} // namespace GUI
