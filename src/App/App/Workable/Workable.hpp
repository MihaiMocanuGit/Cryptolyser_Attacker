#pragma once
#include "../JobI/JobI.hpp"
#include "../WorkloadManager/WorkloadManager.hpp"

#include <memory>

namespace App
{
class Workable
{
  protected:
    WorkloadManager &m_workloadManager;

  public:
    explicit Workable(WorkloadManager &workloadManager) : m_workloadManager {workloadManager} {}

    [[nodiscard]] virtual std::unique_ptr<JobI> job() const = 0;

    virtual ~Workable() = default;
};
} // namespace App
