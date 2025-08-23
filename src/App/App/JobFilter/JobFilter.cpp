#include "JobFilter.hpp"

#include "Study/SerializerManager/SerializerManager.hpp"

namespace App
{
JobFilter::JobFilter(const JobFilter::Buffers &buffers) : JobI {}
{
    input.lb = buffers.lb;
    input.ub = buffers.ub;
    input.savePath = buffers.savePath;
    input.loadPath = buffers.loadPath;
}

[[nodiscard]] JobFilter::ExitStatus_e JobFilter::invoke(std::stop_token stoken)
{
    if (stoken.stop_requested())
        return ExitStatus_e::KILLED;
    std::cout << "Started Filter job.\n";
    std::cout << "Loading timing data...\n";
    const auto metadata = SerializerManager::loadTimingMetadata(input.loadPath);
    // TODO: Find a better way to deal with this common pattern. I only need conditional variable
    // initialization and its type definition
    if (metadata.knownKey)
    {
        TimingData<true> timingData {metadata.dataSize, metadata.key};
        if (stoken.stop_requested())
            return ExitStatus_e::KILLED;
        SerializerManager::loadRaw(input.loadPath, timingData);

        if (stoken.stop_requested())
            return ExitStatus_e::KILLED;
        std::cout << "Filtering the timing data...\n";
        Filter::filter<double>(timingData.timing(), [this](double data)
                               { return input.lb <= data and data <= input.ub; });

        if (stoken.stop_requested())
            return ExitStatus_e::KILLED;
        std::cout << "Saving the filtered timing data...\n";
        SerializerManager::saveRaw(input.savePath, timingData);
        std::cout << "Finished Filter job.\n\n";
    }
    else
    {
        TimingData<false> timingData {metadata.dataSize};
        if (stoken.stop_requested())
            return ExitStatus_e::KILLED;
        SerializerManager::loadRaw(input.loadPath, timingData);

        if (stoken.stop_requested())
            return ExitStatus_e::KILLED;
        std::cout << "Filtering the timing data...\n";
        Filter::filter<double>(timingData.timing(), [this](double data)
                               { return input.lb <= data and data <= input.ub; });

        if (stoken.stop_requested())
            return ExitStatus_e::KILLED;
        std::cout << "Saving the filtered timing data...\n";
        SerializerManager::saveRaw(input.savePath, timingData);
        std::cout << "Finished Filter job.\n\n";
    }
    return ExitStatus_e::OK;
}

std::string JobFilter::description() const noexcept
{
    return std::format("Filter - [{}, {}], {} -> {}", input.lb, input.ub, input.loadPath.string(),
                       input.savePath.string());
}

std::unique_ptr<JobI> JobFilter::clone() const { return std::make_unique<JobFilter>(*this); }
} // namespace App
