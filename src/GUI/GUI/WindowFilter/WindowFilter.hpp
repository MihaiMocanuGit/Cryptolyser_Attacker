#pragma once

#include "../WindowI/WindowI.hpp"
#include "../WorkableWindow/WorkableWindow.hpp"
#include "App/JobFilter/JobFilter.hpp"

namespace GUI
{
class WindowFilter : public WindowI, public WorkableWindow
{
  private:
    App::JobFilter::Buffers m_buffers {};

  public:
    WindowFilter(std::string_view name, App::WorkloadManager &workloadManager);

    [[nodiscard]] std::unique_ptr<App::JobI> job() const override;

    void constructWindow() override;
};
} // namespace GUI
