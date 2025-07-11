#include "JobFilter.hpp"

#include "Study/SerializerManager/SerializerManager.hpp"

App::JobFilter::JobFilter(const App::JobFilter::Buffers &buffers,
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
    std::cout << "Loading timing data...\n";
    const auto metadata = SerializerManager::loadTimingMetadata(input.loadPath);
    // TODO: Find a better way to deal with this common pattern. I only need conditional variable
    // initialization and its type definition
    if (metadata.knownKey)
    {
        TimingData<true> timingData {metadata.dataSize, metadata.key};
        SerializerManager::loadRaw(input.loadPath, timingData);
        std::cout << "Filtering the timing data...\n";
        Filter::filter<double>(timingData.timing(), [this](double data)
                               { return input.lb <= data and data <= input.ub; });
        std::cout << "Saving the filtered timing data...\n";
        SerializerManager::saveRaw(input.savePath, timingData);
        std::cout << "Finished Filter job.\n\n";
    }
    else
    {
        TimingData<false> timingData {metadata.dataSize};
        SerializerManager::loadRaw(input.loadPath, timingData);
        std::cout << "Filtering the timing data...\n";
        Filter::filter<double>(timingData.timing(), [this](double data)
                               { return input.lb <= data and data <= input.ub; });
        std::cout << "Saving the filtered timing data...\n";
        SerializerManager::saveRaw(input.savePath, timingData);
        std::cout << "Finished Filter job.\n\n";
    }
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
