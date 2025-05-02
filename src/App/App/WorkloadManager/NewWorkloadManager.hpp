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
class NewWorkloadManager
{
  public:
    enum class States
    {
        NOT_STARTED, // State appears:
                     //      1. Right after object init.
                     //      2. After the whole workqueue has been processed. That is, the manager
                     //      had been "rearmed" after it was in a FINISHED state.

        BUSY, // State while processing the work queue

        PAUSE_AFTER_THIS, // Got a pause command, will pause the processing after the current job

        PAUSED, // Finished processing the current job (that got PAUSE_AFTER_THIS), so we're
                // stopping. NOTE that PAUSE can only appear like so: BUSY -> PAUSE_AFTER_THIS ->
                // PAUSED

        FORCEFULLY_STOPPED, // g_continueRunning has been set to False;

        FINISHED, // All cloneJobs have been processed

        INVALID, // Something really wrong happened...
    };

  private:
    std::thread m_worker;

    std::vector<std::unique_ptr<JobI>> m_workload {};

    mutable std::mutex m_workloadMutex;

    std::atomic_size_t m_currentJobIndex {0};

    const std::atomic_flag &m_g_continueRunning;

    void m_throwOnInvalidIndex(size_t index) const;

    std::atomic<States> m_currentState {States::NOT_STARTED};

    void m_processWorkload();

  public:
    explicit NewWorkloadManager(const std::atomic_flag &continueRunning);

    States state() const noexcept;

    size_t currentJobIndex() const noexcept;

    size_t size() const noexcept;

    bool addJob(std::unique_ptr<JobI> job);

    bool removeJob(size_t jobIndex);

    bool swapJobs(size_t jobIndex1, size_t jobIndex2);

    bool start(size_t firstJobIndex = 0);

    bool resume();

    bool pauseAfterJob();

    bool rearmManager();

    [[nodiscard]] std::vector<std::unique_ptr<JobI>> cloneJobs() const;

    [[nodiscard]] std::vector<std::string> jobDescriptions() const;
};

} // namespace App
