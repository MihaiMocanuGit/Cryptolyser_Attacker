#include "JobFilter.hpp"

App::JobFilter::JobFilter(const App::BuffersFilter &buffers,
                          const std::atomic_flag &continueRunning)
    : JobI {continueRunning}
{
    input.lb = buffers.lb;
    input.ub = buffers.ub;
    input.savePath = buffers.savePath;
    input.loadPath = buffers.loadPath;
}

void App::JobFilter::operator()()
{
    TimingData<false> timingData {0};
    SerializerManager::loadRaw(input.loadPath, timingData);
    Filter::filter<double>(timingData.timing(),
                           [this](double data) { return input.lb <= data and data <= input.ub; });
    SerializerManager::saveRaw(input.savePath, timingData);
}

std::string App::JobFilter::description() const noexcept
{
    return std::format("Filter - [{}, {}], {} -> {}", input.lb, input.ub, input.loadPath.string(),
                       input.savePath.string());
}

std::unique_ptr<App::JobI> App::JobFilter::clone() const
{
    return std::make_unique<JobFilter>(*this);
}
