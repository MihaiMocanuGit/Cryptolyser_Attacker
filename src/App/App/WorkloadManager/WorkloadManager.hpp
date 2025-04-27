#pragma once

#include "../JobI/JobI.hpp"

#include <atomic>
#include <cassert>
#include <deque>
#include <format>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace App
{
class WorkloadManager
{
  private:
    std::thread m_worker;
    std::deque<std::unique_ptr<JobI>> m_workload {};
    std::mutex m_workloadsMutex;
    std::atomic_flag m_busy {false};
    std::atomic<size_t> m_totalJobsCount {0};
    std::atomic<size_t> m_processedJobsCount {0};

    const std::atomic_flag &m_continueRunning;

    void panicClean();

  public:
    explicit WorkloadManager(const std::atomic_flag &continueRunning);

    void push(std::unique_ptr<JobI> job);

    std::vector<std::string> queuedJobDescriptions() noexcept;

    void start();

    [[nodiscard]] size_t totalJobsCount() const noexcept;

    [[nodiscard]] size_t processedJobsCount() const noexcept;

    [[nodiscard]] bool busy() const noexcept;

    void tryForcefulStop() noexcept;

    [[nodiscard]] const std::atomic_flag &continueRunning() const noexcept
    {
        return m_continueRunning;
    }
};
} // namespace App
