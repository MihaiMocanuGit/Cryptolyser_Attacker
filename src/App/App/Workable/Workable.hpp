#pragma once
#include "../JobI/JobI.hpp"
#include "../WorkloadManager/NewWorkloadManager.hpp"

#include <memory>

namespace App
{
class Workable
{
  protected:
    NewWorkloadManager &m_workloadManager;

  public:
    explicit Workable(NewWorkloadManager &workloadManager) : m_workloadManager {workloadManager} {}

    [[nodiscard]] virtual std::unique_ptr<JobI> job() const = 0;

    virtual ~Workable() = default;
};
} // namespace App
