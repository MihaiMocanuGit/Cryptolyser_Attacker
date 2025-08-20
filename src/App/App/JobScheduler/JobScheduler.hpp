#pragma once

#include "App/JobI/JobI.hpp"

#include <thread>
#include <vector>
namespace App
{
// The WorkloadManager hold a queue of Jobs. When the manager is resumed (also implies started) it
// will create a new thread on which all the jobs from said queue will be sequentially run.
class JobScheduler
{
  public:
    enum class States
    {
        NOT_STARTED,
        RUNNING,
        PAUSED,

    };

  private:
    std::atomic<States> m_state;
    // A substate of PLAYING signifing that after the current job finishes, we must pause.
    // It's not directly included into States as it will introduce unwanted complexity for a
    // behaviour that is identical to PLAYING almost everywhere.
    std::atomic_bool m_pausePending {false};

    std::vector<std::unique_ptr<JobI>> m_jobqueue {};
    std::atomic_size_t m_currentJobIndex {npos};
    // using this aditional variable removes the need to lock the workqueue for every size query.
    std::atomic_size_t m_jobqueueSize {0};
    mutable std::mutex m_jobqueueMtx;

    std::jthread m_worker;
    std::stop_source m_stopSource;

    void m_resetJobqueue() noexcept;

    void m_startWorker(size_t startAtJob) noexcept;

    void m_assertGoodBehaviour() const noexcept;

  public:
    static constexpr size_t npos {static_cast<size_t>(-1)};

    JobScheduler() = default;

    // CRUD functionality
    // TODO: Inline these UI helpers here in the header? (Ui helpers = all functions of the form
    // "type can<Action>(params)")
    bool canAddJob(const std::unique_ptr<JobI> &job) const noexcept;
    bool addJob(std::unique_ptr<JobI> &&job);

    bool canInsertJob(size_t jobIndex, const std::unique_ptr<JobI> &job) const noexcept;
    bool insertJob(size_t jobIndex, std::unique_ptr<JobI> &&job);

    bool canRemoveJob(size_t jobIndex) const noexcept;
    bool removeJob(size_t jobIndex);

    bool canUpdateJob(size_t jobIndex, const std::unique_ptr<JobI> &job) const noexcept;
    bool updateJob(size_t jobIndex, std::unique_ptr<JobI> &&job);

    bool canCloneJob(size_t jobIndex) const noexcept;
    [[nodiscard]] std::unique_ptr<JobI> cloneJob(size_t jobIndex) const;

    // Helpful extended CRUD functionality
    bool canSwapJobs(size_t jobIndex1, size_t jobIndex2) const noexcept;
    bool swapJobs(size_t jobIndex1, size_t jobIndex2);

    bool canRemoveAllJobs() const noexcept;
    bool removeAllJobs();

    bool canCloneAllJobs() const noexcept;
    [[nodiscard]] std::vector<std::unique_ptr<JobI>> cloneAllJobs() const;

    // WorkloadManager API
    bool canRun() const noexcept;
    bool run() noexcept;

    bool canPauseAfter() const noexcept;
    bool pauseAfter() noexcept;

    bool canResume() const noexcept;
    bool resume() noexcept;

    bool canKill() const noexcept;
    bool kill();

    // State Visible Statistics
    States state() const noexcept;
    size_t currentJobIndex() const noexcept;
    size_t jobqueueSize() const noexcept;
    std::string jobDescription(size_t jobIndex) const;
    [[nodiscard]] std::vector<std::string> allJobDescriptions() const;
};
} // namespace App
