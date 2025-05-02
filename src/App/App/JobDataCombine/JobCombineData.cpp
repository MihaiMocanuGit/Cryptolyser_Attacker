#include "JobCombineData.hpp"

App::JobCombineData::JobCombineData(const App::BuffersCombineData &buffers,
                                    const std::atomic_flag &continueRunning)
    : JobI {continueRunning}
{
    input.savePath = buffers.savePath;
    for (const auto &path : buffers.loadPaths)
        input.loadPaths.emplace_back(path);
}

void App::JobCombineData::operator()()
{
    if (input.onlyMetrics)
    {
        TimingData<false, MetricsData<double>> timingData {0};
        for (const auto &loadPath : input.loadPaths)
            SerializerManager::loadRaw(loadPath, timingData);
        SerializerManager::saveRaw(input.savePath, timingData);
    }
    else
    {
        TimingData<false, SampleData<double>> timingData {0};
        for (const auto &loadPath : input.loadPaths)
            SerializerManager::loadRaw(loadPath, timingData);
        SerializerManager::saveRaw(input.savePath, timingData);
    }
}

std::string App::JobCombineData::description() const noexcept
{
    std::string description {"CombineData - SavePath: " + input.savePath.string() + "LoadPaths: "};
    for (const std::filesystem::path &path : input.loadPaths)
        description += path.string() + " ";
    description.pop_back(); // remove the last char which is ' '.
    return description;
}

std::unique_ptr<App::JobI> App::JobCombineData::clone() const
{
    return std::make_unique<JobCombineData>(*this);
}
