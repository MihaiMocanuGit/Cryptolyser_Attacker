#include "WorkloadManager.hpp"

void App::WorkloadManager::panicClean()
{
    std::lock_guard<std::mutex> workloadLock {m_workloadsMutex};
    m_workload = std::deque<std::unique_ptr<JobI>> {};
    m_busy.store(false);
    m_totalJobsCount = 0;
    m_processedJobsCount = 0;
}

App::WorkloadManager::WorkloadManager(const std::atomic_bool &continueRunning)
    : m_continueRunning {continueRunning}
{
}

void App::WorkloadManager::push(std::unique_ptr<JobI> job)
{
    std::lock_guard<std::mutex> workloadsLock {m_workloadsMutex};
    m_workload.push_back(std::move(job));
    m_totalJobsCount++;
}

std::vector<std::string> App::WorkloadManager::queuedJobDescriptions() noexcept
{
    std::vector<std::string> descriptions;
    std::lock_guard<std::mutex> workloadLock {m_workloadsMutex};
    descriptions.reserve(m_workload.size());
    for (const auto &job : m_workload)
        descriptions.push_back(job->description());
    return descriptions;
}

void App::WorkloadManager::start()
{
    const auto processWorkQueue = [this]()
    {
        assert(m_busy.load() == false);
        m_busy.store(true);
        try
        {
            while (true)
            {
                std::unique_ptr<JobI> nextJob;
                {
                    std::lock_guard<std::mutex> workloadLock {m_workloadsMutex};
                    if (not m_workload.empty())
                    {
                        nextJob = std::move(m_workload.front());
                        m_workload.pop_front();
                    }
                    else // work queue is empty so we finished all the work
                    {
                        break;
                    }
                }
                nextJob->operator()();
                m_processedJobsCount++;
            }
        }
        catch (const std::runtime_error &err)
        {
            std::cerr << err.what() << std::endl;
            panicClean();
            return;
        }
        catch (const std::exception &excp)
        {
            panicClean();
            return;
        }
        m_busy.store(false);
    };
    if (not m_busy.load())
    {
        m_worker = std::thread {processWorkQueue};
        m_worker.detach();
    }
}

size_t App::WorkloadManager::totalJobsCount() const noexcept { return m_totalJobsCount; }

size_t App::WorkloadManager::processedJobsCount() const noexcept { return m_processedJobsCount; }

bool App::WorkloadManager::busy() const noexcept { return m_busy.load(); }

void App::WorkloadManager::tryForcefulStop() noexcept { panicClean(); }
