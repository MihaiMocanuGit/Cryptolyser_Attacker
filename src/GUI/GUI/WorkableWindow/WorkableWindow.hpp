#pragma once
#include "App/JobI/JobI.hpp"
#include "App/WorkloadManager/WorkloadManager.hpp"

#include <memory>

namespace GUI
{
class WorkableWindow
{
  protected:
    App::WorkloadManager &m_workloadManager;

  public:
    explicit WorkableWindow(App::WorkloadManager &workloadManager)
        : m_workloadManager {workloadManager}
    {
    }

    [[nodiscard]] virtual std::unique_ptr<App::JobI> job() const = 0;

    virtual ~WorkableWindow() = default;
};
} // namespace GUI
