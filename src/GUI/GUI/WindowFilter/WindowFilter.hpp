#pragma once

#include "../WindowI/WindowI.hpp"
#include "App/JobFilter/JobFilter.hpp"
#include "App/Workable/Workable.hpp"

namespace GUI
{
class WindowFilter : public WindowI, public App::Workable
{
  private:
    App::BuffersFilter m_buffers {};

  public:
    WindowFilter(std::string_view name, App::WorkloadManager &workloadManager);

    [[nodiscard]] std::unique_ptr<App::JobI> job() const override;

    void constructWindow() override;
};
} // namespace GUI
