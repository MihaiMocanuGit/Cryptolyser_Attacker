#include "JobScheduler.hpp"

#include <cassert>
#include <iostream>

namespace App
{

#define ASSERT_SINGLE m_assertGoodBehaviour();

#define ASSERT_START                                                                               \
    m_assertGoodBehaviour();                                                                       \
    { // a scope bracket is needed so that m_assertGoodBehaviour doesn't try to lock a mutex already
      // in use, that would've correctly went out of scope by the end.

#define ASSERT_END                                                                                 \
    }                                                                                              \
    m_assertGoodBehaviour();

void JobScheduler::m_startWorker(size_t startAtJob) noexcept
{
    const auto logic = [this](std::stop_token stoken, size_t startAtJob)
    {
        ASSERT_START
        m_state = States::RUNNING;
        for (m_currentJobIndex = startAtJob;
             // The state can be externally changed (or internally due to an exception inside the
             // job). Thus it is checked after every iteration.
             m_currentJobIndex < m_jobqueueSize and m_state == States::RUNNING and
             not m_pausePending;
             ++m_currentJobIndex)
        {
            ASSERT_START
            JobI::ExitStatus_e status;
            // cloning the job so that we don't consume the mutex during the whole job operation
            std::unique_ptr<JobI> jobClone;
            {
                std::lock_guard lock {m_jobqueueMtx};
                jobClone = m_jobqueue[m_currentJobIndex]->clone();
            }
            try
            {
                status = jobClone->invoke(stoken);
                if (status == JobI::ExitStatus_e::KILLED)
                {
                    std::cerr << "Job killed by user: " << m_currentJobIndex << std::endl;
                    m_state = States::PAUSED;
                    break;
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "Unknown Exception: " << e.what() << std::endl;
                m_state = States::PAUSED;
                break;
            }
            ASSERT_END
        }
        assert(m_state != States::NOT_STARTED);
        if (m_state == States::RUNNING)
        {
            if (m_pausePending)
                m_state = States::PAUSED;
            else // we finished all jobs succesfully
                m_resetJobqueue();
        }
        else // We either got an error or the user killed the current job. We prematurely pause to
             // let the user deal with it accordingly.
            std::cerr << "FORCEFULLY PAUSED THE JOB SCHEDULER...\n"
                      << "\t(resuming will restart the current job again.)" << std::endl;
        ASSERT_END
    };
    m_worker = std::jthread {logic, m_stopSource.get_token(), startAtJob};
}

void JobScheduler::m_resetJobqueue() noexcept
{
    ASSERT_START
    m_state = States::NOT_STARTED;
    m_pausePending = false;
    m_currentJobIndex = npos;
    ASSERT_END
}

void JobScheduler::m_assertGoodBehaviour() const noexcept
{
    {
        std::lock_guard lock {m_jobqueueMtx};
        assert(m_jobqueueSize == m_jobqueue.size());
    }
    switch (m_state)
    {
        case States::NOT_STARTED:
            assert(m_currentJobIndex == npos);

            assert(m_pausePending == false);
            break;
        case States::PAUSED:
            assert(m_jobqueueSize > 0);
            assert(m_currentJobIndex < m_jobqueueSize);
            assert(m_jobqueueSize != npos);

            assert(m_pausePending == false);
            break;
        case States::RUNNING:
            assert(m_jobqueueSize > 0);
            assert(m_currentJobIndex < m_jobqueueSize);
            assert(m_jobqueueSize != npos);
            break;
    }
}

bool JobScheduler::canAddJob(const std::unique_ptr<JobI> &job) const noexcept
{
    return job != nullptr;
}

bool JobScheduler::addJob(std::unique_ptr<JobI> &&job)
{
    if (not canAddJob(job))
        return false;
    ASSERT_START
    std::lock_guard lock {m_jobqueueMtx};
    m_jobqueue.emplace_back(std::move(job));
    m_jobqueueSize++;
    ASSERT_END
    return true;
}

bool JobScheduler::canInsertJob(size_t jobIndex, const std::unique_ptr<JobI> &job) const noexcept
{
    return jobIndex <= m_jobqueueSize and job != nullptr;
}

bool JobScheduler::insertJob(size_t jobIndex, std::unique_ptr<JobI> &&job)
{
    if (not canInsertJob(jobIndex, job))
        return false;
    ASSERT_START
    switch (m_state)
    {
        case States::PAUSED:
            [[fallthrough]];
        case States::RUNNING:
            if (jobIndex <= m_currentJobIndex)
                m_currentJobIndex++;
            [[fallthrough]];
        case States::NOT_STARTED:
            // TODO: perhaps lock should happen before m_currentJobIndex++ . If so, update for all
            std::lock_guard lock {m_jobqueueMtx};
            m_jobqueue.insert(m_jobqueue.begin() + jobIndex, std::move(job));
            m_jobqueueSize++;
            break;
    }
    ASSERT_END
    return true;
}

bool JobScheduler::canRemoveJob(size_t jobIndex) const noexcept
{
    if (jobIndex >= m_jobqueueSize)
        return false;
    switch (m_state)
    {
        case States::RUNNING:
            if (jobIndex == m_currentJobIndex)
                return false;
            [[fallthrough]];
        case States::PAUSED:
            [[fallthrough]];
        case States::NOT_STARTED:
            break;
    }
    return true;
}

bool JobScheduler::removeJob(size_t jobIndex)
{
    if (not canRemoveJob(jobIndex))
        return false;
    ASSERT_START
    switch (m_state)
    {
        case States::RUNNING:
            if (jobIndex < m_currentJobIndex)
                m_currentJobIndex--;
            [[fallthrough]];
        case States::PAUSED:
            // if the current job is the last one in the queue and we remove it, then we must bring
            // back the jobqueue to NOT_STARTED state.
            if (jobIndex == m_jobqueueSize - 1)
                m_resetJobqueue();
            [[fallthrough]];
        case States::NOT_STARTED:
            std::lock_guard lock {m_jobqueueMtx};
            m_jobqueue.erase(m_jobqueue.begin() + jobIndex);
            m_jobqueueSize--;
            break;
    }
    ASSERT_END
    return true;
}

bool JobScheduler::canUpdateJob(size_t jobIndex, const std::unique_ptr<JobI> &job) const noexcept
{
    return canRemoveJob(jobIndex) && canInsertJob(jobIndex, job);
}

bool JobScheduler::updateJob(size_t jobIndex, std::unique_ptr<JobI> &&job)
{
    if (not canUpdateJob(jobIndex, job))
        return false;
    ASSERT_START
    switch (m_state)
    {
        case States::PAUSED:
            [[fallthrough]];
        case States::RUNNING:
            [[fallthrough]];
        case States::NOT_STARTED:
            std::lock_guard lock {m_jobqueueMtx};
            m_jobqueue[jobIndex] = std::move(job);
            break;
    }
    ASSERT_END
    return true;
}

bool JobScheduler::canCloneJob(size_t jobIndex) const noexcept { return jobIndex < m_jobqueueSize; }

std::unique_ptr<JobI> JobScheduler::cloneJob(size_t jobIndex) const
{
    if (not canCloneJob(jobIndex))
        return {nullptr};
    ASSERT_SINGLE
    std::lock_guard lock {m_jobqueueMtx};
    return m_jobqueue[jobIndex]->clone();
}

bool JobScheduler::canSwapJobs(size_t jobIndex1, size_t jobIndex2) const noexcept
{
    bool cond {jobIndex1 < m_jobqueueSize and jobIndex1 != m_currentJobIndex};
    cond = cond and (jobIndex2 < m_jobqueueSize and jobIndex2 != m_currentJobIndex);
    return cond;
}

bool JobScheduler::swapJobs(size_t jobIndex1, size_t jobIndex2)
{
    if (not canSwapJobs(jobIndex1, jobIndex2))
        return false;
    ASSERT_START
    switch (m_state)
    {
        case States::PAUSED:
            [[fallthrough]];
        case States::RUNNING:
            [[fallthrough]];
        case States::NOT_STARTED:
            if (jobIndex1 == jobIndex2)
                break;
            std::lock_guard lock {m_jobqueueMtx};
            std::swap(m_jobqueue[jobIndex1], m_jobqueue[jobIndex2]);
            break;
    }
    ASSERT_END
    return true;
}

bool JobScheduler::canRemoveAllJobs() const noexcept
{
    // this is useful for the UI because a removeAll button could disappear (/be grayed out) when
    // the queue is empty.
    switch (m_state)
    {
        case States::RUNNING:
            return m_jobqueueSize > 1;
        default:
            return m_jobqueueSize > 0;
    }
}

bool JobScheduler::removeAllJobs()
{

    if (not canRemoveAllJobs())
        return false;
    ASSERT_START
    std::lock_guard lock {m_jobqueueMtx};
    switch (m_state)
    {
        case States::RUNNING:
            // erasing before and after the current job.
            m_jobqueue.erase(m_jobqueue.begin(), m_jobqueue.begin() + m_currentJobIndex);
            // The case vec.erase(vec.end(), vec.end()) is valid (and no-op)
            m_jobqueue.erase(m_jobqueue.begin() + m_currentJobIndex + 1, m_jobqueue.end());
            m_jobqueueSize = 1;
            break;
        case States::PAUSED:
            [[fallthrough]];
        case States::NOT_STARTED:
            m_jobqueue.clear();
            m_jobqueueSize = 0;
            m_resetJobqueue();
            break;
    }
    ASSERT_END
    return true;
}

bool JobScheduler::canCloneAllJobs() const noexcept
{
    // no innate reason for this behaviour, it would just be consistent with
    // canRemoveAllFutureJobs(). That is, you cannot clone/remove an empty queue.
    return m_jobqueueSize > 0;
}

[[nodiscard]] std::vector<std::unique_ptr<JobI>> JobScheduler::cloneAllJobs() const
{
    if (not canCloneAllJobs())
        return {};

    std::vector<std::unique_ptr<JobI>> jobs;
    ASSERT_START
    std::lock_guard lock {m_jobqueueMtx};
    for (const auto &job : m_jobqueue)
        jobs.push_back(job->clone());
    ASSERT_END
    return jobs;
}

bool JobScheduler::canRun() const noexcept
{
    // TODO: Yay or Nay for this coding style? I kind of like this, but it does feel a little out of
    // place.
    // TODO: Decide if the previous TODO was really a TODO.
    // TODO: Repeat ad infinitum
    switch (m_state)
    {
        case States::NOT_STARTED:
            return m_jobqueueSize > 0;
        default:
            return false;
    }
}

bool JobScheduler::run() noexcept
{
    if (not canRun())
        return false;
    ASSERT_START
    assert(m_currentJobIndex == npos);
    m_startWorker(0);
    ASSERT_END
    return true;
}

bool JobScheduler::canPauseAfter() const noexcept
{
    switch (m_state)
    {
        case States::RUNNING:
            // cannot logically pause after the last job.
            return m_currentJobIndex < m_jobqueueSize - 1;
        default:
            return false;
    }
}

bool JobScheduler::pauseAfter() noexcept
{
    if (not canPauseAfter())
        return false;
    ASSERT_START
    m_pausePending = true;
    ASSERT_END
    return true;
}

bool JobScheduler::canResume() const noexcept
{
    switch (m_state)
    {
        case States::PAUSED:
            return true;
        default:
            return false;
    }
}

bool JobScheduler::resume() noexcept
{
    if (not canResume())
        return false;
    ASSERT_START
    m_startWorker(m_currentJobIndex);
    ASSERT_END
    return true;
}

bool JobScheduler::canKill() const noexcept
{
    switch (m_state)
    {
        case States::RUNNING:
            return m_stopSource.stop_possible();
        default:
            return false;
    }
}

bool JobScheduler::kill()
{
    if (not canKill())
        return false;
    ASSERT_START
    m_stopSource.request_stop();
    ASSERT_END
    return true;
}

JobScheduler::States JobScheduler::state() const noexcept { return m_state; }

size_t JobScheduler::currentJobIndex() const noexcept { return m_currentJobIndex; }

size_t JobScheduler::jobqueueSize() const noexcept { return m_jobqueueSize; }

std::string JobScheduler::jobDescription(size_t jobIndex) const
{
    if (jobIndex < m_jobqueueSize)
    {
        std::lock_guard lock {m_jobqueueMtx};
        return m_jobqueue[jobIndex]->description();
    }
    return "";
}

[[nodiscard]] std::vector<std::string> JobScheduler::allJobDescriptions() const
{
    std::vector<std::string> descriptions;
    descriptions.reserve(m_jobqueueSize);
    std::lock_guard lock {m_jobqueueMtx};
    for (const auto &job : m_jobqueue)
        descriptions.push_back(job->description());
    return descriptions;
}

} // namespace App
