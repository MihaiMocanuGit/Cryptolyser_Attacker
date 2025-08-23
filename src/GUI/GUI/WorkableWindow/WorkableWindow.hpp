#pragma once
#include "App/JobI/JobI.hpp"
#include "App/JobScheduler/JobScheduler.hpp"

#include <memory>

namespace GUI
{
class WorkableWindow
{
  protected:
    App::JobScheduler &m_jobScheduler;

  public:
    explicit WorkableWindow(App::JobScheduler &jobScheduler) : m_jobScheduler {jobScheduler} {}

    [[nodiscard]] virtual std::unique_ptr<App::JobI> job() const = 0;

    virtual ~WorkableWindow() = default;
};
} // namespace GUI
