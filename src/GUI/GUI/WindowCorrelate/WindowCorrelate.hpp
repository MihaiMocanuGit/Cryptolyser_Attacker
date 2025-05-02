#pragma once

#include "../WindowI/WindowI.hpp"
#include "App/JobCorrelate/JobCorrelate.hpp"
#include "App/Workable/Workable.hpp"

namespace GUI
{
class WindowCorrelate : public WindowI, public App::Workable
{
  private:
    App::BuffersCorrelate m_buffers {};

  public:
    WindowCorrelate(std::string_view name, App::NewWorkloadManager &workloadManager);

    [[nodiscard]] std::unique_ptr<App::JobI> job() const override;

    void constructWindow() override;
};
} // namespace GUI
