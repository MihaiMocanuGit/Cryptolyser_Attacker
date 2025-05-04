//

#include "NewWorkloadManager.hpp"

namespace App
{
NewWorkloadManager::NewWorkloadManager(const std::atomic_bool &continueRunning)
    : m_g_continueRunning {continueRunning}
{
}

NewWorkloadManager::States NewWorkloadManager::state() const noexcept
{
    return m_currentState.load();
}

size_t NewWorkloadManager::currentJobIndex() const noexcept { return m_currentJobIndex.load(); }

size_t NewWorkloadManager::size() const noexcept
{
    std::lock_guard lock {m_workloadMutex};
    return m_workload.size();
}

void NewWorkloadManager::m_throwOnInvalidIndex(size_t index) const
{
    if (index >= size())
        throw std::out_of_range("WorkloadManager: invalid job index access.");
}

bool NewWorkloadManager::addJob(std::unique_ptr<JobI> job)
{
    const auto add = [&job, this]()
    {
        std::lock_guard lock {m_workloadMutex};
        m_workload.emplace_back(std::move(job));
    };

    switch (m_currentState)
    {
        case States::NOT_STARTED:
            [[fallthrough]];
        case States::BUSY:
            [[fallthrough]];
        case States::PAUSE_AFTER_THIS:
            [[fallthrough]];
        case States::PAUSED:
            add();
            return true;
        case States::FINISHED:
            return false;
        case States::FORCEFULLY_STOPPED:
            return false;
        case States::INVALID:
            throw std::runtime_error("WorkloadManager: Invalid State");
    }
    throw std::runtime_error("WorkloadManager: Invalid State");
}

bool NewWorkloadManager::removeJob(size_t jobIndex)
{
    m_throwOnInvalidIndex(jobIndex);

    switch (m_currentState)
    {
        case States::NOT_STARTED:
        {
            std::lock_guard lock {m_workloadMutex};
            m_workload.erase(m_workload.begin() + jobIndex);
            return true;
        }
        case States::BUSY:
            [[fallthrough]];
        case States::PAUSE_AFTER_THIS:
            if (jobIndex == m_currentJobIndex.load())
                return false;
            [[fallthrough]];
        case States::PAUSED:
        {
            if (jobIndex < m_currentJobIndex.load())
                return false;
            //            // If we remove the last element, we need to specify that we finished the
            //            queuea.
            //            // Otherwise, while processing the queue, we might get an out of range
            //            error from time
            //            // to time, depending on the current thread synchronisation
            //            if (jobIndex == size() - 1 and jobIndex == m_currentJobIndex.load())
            //            {
            //                m_currentState = States::FINISHED;
            //                std::lock_guard lock {m_workloadMutex};
            //                m_workload.erase(m_workload.begin() + jobIndex);
            //                return true;
            //            }
            std::lock_guard lock {m_workloadMutex};
            m_workload.erase(m_workload.begin() + jobIndex);
            return true;
        }
        case States::FINISHED:
            return false;
        case States::FORCEFULLY_STOPPED:
            return false;
        case States::INVALID:
            throw std::runtime_error("WorkloadManager: Invalid State");
    }
    throw std::runtime_error("WorkloadManager: Invalid State");
}

bool NewWorkloadManager::removeAllPossibleJobs()
{
    switch (m_currentState)
    {
        case States::NOT_STARTED:
        {
            std::lock_guard lock {m_workloadMutex};
            m_workload.erase(m_workload.begin(), m_workload.end());
            return true;
        }
        case States::BUSY:
            [[fallthrough]];
        case States::PAUSE_AFTER_THIS:
        {
            std::lock_guard lock {m_workloadMutex};
            m_workload.erase(m_workload.begin() + m_currentJobIndex.load() + 1, m_workload.end());
            return true;
        }
        case States::PAUSED:
        {
            std::lock_guard lock {m_workloadMutex};
            m_workload.erase(m_workload.begin() + m_currentJobIndex.load(), m_workload.end());
            return true;
        }
        case States::FINISHED:
            return false;
        case States::FORCEFULLY_STOPPED:
            return false;
        case States::INVALID:
            throw std::runtime_error("WorkloadManager: Invalid State");
    }
    throw std::runtime_error("WorkloadManager: Invalid State");
}

bool NewWorkloadManager::swapJobs(size_t jobIndex1, size_t jobIndex2)
{
    m_throwOnInvalidIndex(jobIndex1);
    m_throwOnInvalidIndex(jobIndex2);

    switch (m_currentState)
    {
        case States::NOT_STARTED:
        {
            std::lock_guard lock {m_workloadMutex};
            std::swap(m_workload[jobIndex1], m_workload[jobIndex2]);
            return true;
        }
        case States::BUSY:
            [[fallthrough]];
        case States::PAUSE_AFTER_THIS:
            [[fallthrough]];
        case States::PAUSED:
        {
            if (jobIndex1 <= m_currentJobIndex.load() or jobIndex2 <= m_currentJobIndex.load())
                return false;
            std::lock_guard lock {m_workloadMutex};
            std::swap(m_workload[jobIndex1], m_workload[jobIndex2]);
            return true;
        }
        case States::FINISHED:
            return false;
        case States::FORCEFULLY_STOPPED:
            return false;
        case States::INVALID:
            throw std::runtime_error("WorkloadManager: Invalid State");
    }
    throw std::runtime_error("WorkloadManager: Invalid State");
}

bool NewWorkloadManager::start(size_t firstJobIndex)
{
    m_throwOnInvalidIndex(firstJobIndex);

    switch (m_currentState)
    {
        case States::NOT_STARTED:
        {
            m_currentJobIndex = firstJobIndex;
            m_currentState = States::BUSY;
            m_worker = std::thread {&NewWorkloadManager::m_processWorkload, this};
            m_worker.detach();
            return true;
        }
        case States::BUSY:
            [[fallthrough]];
        case States::PAUSE_AFTER_THIS:
            [[fallthrough]];
        case States::PAUSED:
            [[fallthrough]];
        case States::FORCEFULLY_STOPPED:
            [[fallthrough]];
        case States::FINISHED:
            return false;
        case States::INVALID:
            throw std::runtime_error("WorkloadManager: Invalid State");
    }
    throw std::runtime_error("WorkloadManager: Invalid State");
}

void NewWorkloadManager::m_processWorkload()
{
    if (size() == 0) // Start was called with an empty queue, rearm and exit.
    {
        m_currentState = States::NOT_STARTED;
        return;
    }
    // Process the work queue as long as we don't get an external stop/pause command
    // Likewise, if the whole queue was processed, then also stop.
    while (m_g_continueRunning.load() and
           (m_currentState == States::BUSY or m_currentState == States::PAUSE_AFTER_THIS))
    {
        std::unique_ptr<JobI> job;
        {
            // We copy the current job so that we don't keep this mutex alive for too long.
            std::lock_guard workloadLock {m_workloadMutex};
            if (m_currentJobIndex.load() < m_workload.size())
                job = m_workload.at(m_currentJobIndex.load())->clone();
            else
                assert(m_currentJobIndex.load() <
                       m_workload.size()); // If we arrive here, this will fail.
        }
        // start the job
        try
        {
            job->operator()();
            // the job has finished, so we can now increment the index
            m_currentJobIndex.fetch_add(1);
        }
        catch (const std::exception &err)
        {
            std::cerr << "WorkloadManager job error: " << err.what() << '\n';
            std::cerr << "\tPausing Manager...\n" << std::endl;
            m_currentState = States::PAUSED;
        }
        // if the manager is paused at the last job, and we remove it, then m_currentJobIndex will
        // be one over the new size().
        if (m_currentJobIndex.load() >= size())
        {
            m_currentState = States::FINISHED;
            assert(m_currentJobIndex <= size() + 1);
            m_currentJobIndex.store(size());
        }
        else if (m_currentState == States::PAUSE_AFTER_THIS)
            m_currentState = States::PAUSED;
    }
    if (not m_g_continueRunning.load())
        m_currentState.store(States::FORCEFULLY_STOPPED);

    switch (m_currentState)
    {
        case States::PAUSED: // Needs to be resumed from another function.
            return;
        case States::FORCEFULLY_STOPPED: // Just exit, the program should close after.
            return;
        case States::FINISHED: // The Manager needs to be "rearmed" to start again.
            return;
        case States::PAUSE_AFTER_THIS: // This should not happen at all.
            [[fallthrough]];
        case States::BUSY: // This should not happen at all.
            [[fallthrough]];
        case States::NOT_STARTED: // This should not happen at all.
            m_currentState = States::INVALID;
            [[fallthrough]];
        case States::INVALID:
            throw std::runtime_error("WorkloadManager: Invalid State");
    }
    throw std::runtime_error("WorkloadManager: Invalid State");
}

bool NewWorkloadManager::resume()
{
    switch (m_currentState)
    {
        case States::PAUSED:
        {
            if (m_currentJobIndex < size())
            {
                m_currentState = States::NOT_STARTED;
                start(m_currentJobIndex);
                return true;
            }
            else
            {
                m_currentState = States::FINISHED;
                return true;
            }
        }
        case States::NOT_STARTED:
            [[fallthrough]];
        case States::BUSY:
            [[fallthrough]];
        case States::PAUSE_AFTER_THIS:
            [[fallthrough]];
        case States::FORCEFULLY_STOPPED:
            [[fallthrough]];
        case States::FINISHED:
            return false;
        case States::INVALID:
            throw std::runtime_error("WorkloadManager: Invalid State");
    }
    throw std::runtime_error("WorkloadManager: Invalid State");
}

bool NewWorkloadManager::pauseAfterJob()
{
    switch (m_currentState)
    {
        case States::BUSY:
            m_currentState = States::PAUSE_AFTER_THIS;
            return true;
        case States::NOT_STARTED:
            [[fallthrough]];
        case States::PAUSE_AFTER_THIS:
            [[fallthrough]];
        case States::PAUSED:
            [[fallthrough]];
        case States::FORCEFULLY_STOPPED:
            [[fallthrough]];
        case States::FINISHED:
            return false;
        case States::INVALID:
            throw std::runtime_error("WorkloadManager: Invalid State");
    }
    throw std::runtime_error("WorkloadManager: Invalid State");
}

bool NewWorkloadManager::rearmManager()
{
    switch (m_currentState)
    {
        case States::FINISHED:
            m_currentState = States::NOT_STARTED;
            m_currentJobIndex = 0;
            return true;
        case States::NOT_STARTED:
            [[fallthrough]];
        case States::BUSY:
            [[fallthrough]];
        case States::PAUSE_AFTER_THIS:
            [[fallthrough]];
        case States::PAUSED:
            [[fallthrough]];
        case States::FORCEFULLY_STOPPED:
            return false;
        case States::INVALID:
            throw std::runtime_error("WorkloadManager: Invalid State");
    }
    throw std::runtime_error("WorkloadManager: Invalid State");
}

std::vector<std::unique_ptr<JobI>> NewWorkloadManager::cloneJobs() const
{
    std::vector<std::unique_ptr<JobI>> copy;
    std::lock_guard lock {m_workloadMutex};
    copy.reserve(m_workload.size());
    for (const auto &job : m_workload)
        copy.emplace_back(job->clone());
    return copy;
}

std::vector<std::string> NewWorkloadManager::jobDescriptions() const
{
    std::vector<std::string> copy;
    std::lock_guard lock {m_workloadMutex};
    copy.reserve(m_workload.size());
    for (const auto &job : m_workload)
        copy.emplace_back(job->description());
    return copy;
}

[[nodiscard]] const std::atomic_bool &NewWorkloadManager::continueRunning() const noexcept
{
    return m_g_continueRunning;
}

NewWorkloadManager::~NewWorkloadManager() { m_currentState = States::FORCEFULLY_STOPPED; }

} // namespace App
