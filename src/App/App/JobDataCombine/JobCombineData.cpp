#include "JobCombineData.hpp"

namespace App
{
JobCombineData::JobCombineData(const JobCombineData::Buffers &buffers) : JobI {}
{
    input.savePath = buffers.savePath;
    for (const auto &path : buffers.loadPaths)
        input.loadPaths.emplace_back(path);
}

[[nodiscard]] JobCombineData::ExitStatus_e JobCombineData::invoke(std::stop_token stoken)
{
    if (stoken.stop_requested())
        return ExitStatus_e::KILLED;
    std::cout << "Started Combine Data job...\n";
    if (input.onlyMetrics)
    {
        TimingData<false, MetricsData<double>> timingData {0};
        std::cout << "Loading timing data...\n";
        for (const auto &loadPath : input.loadPaths)
        {
            if (stoken.stop_requested())
                return ExitStatus_e::KILLED;
            SerializerManager::loadRaw(loadPath, timingData);
        }

        if (stoken.stop_requested())
            return ExitStatus_e::KILLED;
        std::cout << "Saving the combined timing data...\n";
        SerializerManager::saveRaw(input.savePath, timingData);
    }
    else
    {
        TimingData<false, SampleData<double>> timingData {0};
        std::cout << "Loading timing data...\n";
        for (const auto &loadPath : input.loadPaths)
        {
            if (stoken.stop_requested())
                return ExitStatus_e::KILLED;
            SerializerManager::loadRaw(loadPath, timingData);
        }

        if (stoken.stop_requested())
            return ExitStatus_e::KILLED;
        std::cout << "Saving the combined timing data...\n";
        SerializerManager::saveRaw(input.savePath, timingData);
    }
    std::cout << "Finished Combine Data job.\n\n";
    return ExitStatus_e::OK;
}

std::string JobCombineData::description() const noexcept
{
    std::string description {"CombineData - SavePath: " + input.savePath.string() + "LoadPaths: "};
    for (const std::filesystem::path &path : input.loadPaths)
        description += path.string() + " ";
    description.pop_back(); // remove the last char which is ' '.
    return description;
}

std::unique_ptr<JobI> JobCombineData::clone() const
{
    return std::make_unique<JobCombineData>(*this);
}

} // namespace App
