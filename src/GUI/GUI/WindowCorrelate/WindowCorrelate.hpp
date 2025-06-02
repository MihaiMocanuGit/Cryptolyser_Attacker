#pragma once

#include "../WindowI/WindowI.hpp"
#include "../WorkableWindow/WorkableWindow.hpp"
#include "App/JobCorrelate/JobCorrelate.hpp"

namespace GUI
{
class WindowCorrelate : public WindowI, public WorkableWindow
{
  private:
    App::JobCorrelate::Buffers m_buffers {};

  public:
    WindowCorrelate(std::string_view name, App::WorkloadManager &workloadManager);

    [[nodiscard]] std::unique_ptr<App::JobI> job() const override;

    void constructWindow() override;
};
} // namespace GUI
