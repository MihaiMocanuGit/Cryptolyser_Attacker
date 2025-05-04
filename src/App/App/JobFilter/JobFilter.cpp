#include "JobFilter.hpp"

App::JobFilter::JobFilter(const App::BuffersFilter &buffers,
                          const std::atomic_bool &continueRunning)
    : JobI {continueRunning}
{
    input.lb = buffers.lb;
    input.ub = buffers.ub;
    input.savePath = buffers.savePath;
    input.loadPath = buffers.loadPath;
}

void App::JobFilter::operator()()
{
    std::cout << "Started Filter job.\n";
    TimingData<false> timingData {0};
    std::cout << "Loading timing data...\n";
    SerializerManager::loadRaw(input.loadPath, timingData);
    std::cout << "Filtering the timing data...\n";
    Filter::filter<double>(timingData.timing(),
                           [this](double data) { return input.lb <= data and data <= input.ub; });
    std::cout << "Saving the filtered timing data...\n";
    SerializerManager::saveRaw(input.savePath, timingData);
    std::cout << "Finished Filter job.\n\n";
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
