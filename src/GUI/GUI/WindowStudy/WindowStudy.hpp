#pragma once

#include "../WindowI/WindowI.hpp"
#include "../WorkableWindow/WorkableWindow.hpp"
#include "App/JobStudy/JobStudy.hpp"

namespace GUI
{
class WindowStudy : public WindowI, public WorkableWindow
{
  private:
    App::JobStudy::Buffers m_buffers {};

  public:
    WindowStudy(std::string_view name, App::WorkloadManager &workloadManager);

    [[nodiscard]] std::unique_ptr<App::JobI> job() const override;

    void constructWindow() override;
};
} // namespace GUI
