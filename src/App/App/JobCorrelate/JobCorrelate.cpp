#include "JobCorrelate.hpp"

#include "Correlate/Correlate.hpp"
#include "Cryptolyser_Common/connection_data_types.h"
#include "DataProcessing/MetricsData/MetricsData.hpp"
#include "Study/SerializerManager/SerializerManager.hpp"
#include "Study/Study.hpp"
#include "Study/TimingData/TimingData.hpp"

App::JobCorrelate::JobCorrelate(const App::BuffersCorrelate &buffers,
                                const std::atomic_bool &continueRunning)
    : JobI {continueRunning}
{
    input.savePath = buffers.savePath;
    input.victimKeyKnown = buffers.victimKeyKnown;
    input.victimKey = buffers.victimKey;

    for (const auto &path : buffers.victimLoadPaths)
        input.victimLoadPaths.emplace_back(path);
    for (const auto &path : buffers.doppelLoadPaths)
        input.doppelLoadPaths.emplace_back(path);
}

void App::JobCorrelate::operator()()
{
    std::cout << "Started Correlate job...\n";
    // Load all the known victim timing data
    std::vector<TimingData<false, MetricsData<double>>> victimData;
    victimData.reserve(input.victimLoadPaths.size());
    std::cout << "Loading the victim timing data...\n";
    for (const auto &loadPath : input.victimLoadPaths)
    {
        const auto metadata {SerializerManager::loadTimingMetadata(loadPath)};
        assert(not metadata.knownKey);
        TimingData<false, MetricsData<double>> timingData {metadata.dataSize};
        SerializerManager::loadRaw(loadPath, timingData);

        victimData.emplace_back(std::move(timingData));
    }

    // Prepare an empty correlation
    Correlate<MetricsData<double>, MetricsData<double>> correlate {};

    // Load one by one the doppel timing data
    std::cout << "Loading the doppel timing data...\n";
    for (const auto &loadPath : input.doppelLoadPaths)
    {
        const auto metadata {SerializerManager::loadTimingMetadata(loadPath)};
        assert(metadata.knownKey);
        TimingData<true, MetricsData<double>> doppel {metadata.dataSize, metadata.key};
        SerializerManager::loadRaw(loadPath, doppel);

        // Compute the correlations between all the loaded victim timing data and the new doppel
        // timing data.
        for (const auto &victim : victimData)
        {
            Correlate<MetricsData<double>, MetricsData<double>> tmpCorrelate {victim, doppel};

            // Update the main correlation with the new one.
            correlate += tmpCorrelate;
        }
    }

    // save the correlated data to file
    std::string output {};

    const auto orderedCorr {correlate.order()};
    // Write header.
    for (unsigned byte {0}; byte < PACKET_KEY_BYTE_SIZE - 1; ++byte)
        output += std::format("Byte_{}_correlation, Byte_{}_KeyByte, ", byte, byte);
    output += std::format("Byte_{}_correlation, Byte_{}_KeyByte\n", 255, 255);

    SampleData<double> knownKeyCorrStats {};
    // Write the pair of (correlation value , key value)
    for (unsigned value {0}; value < 256; value++)
    {
        for (unsigned byte {0}; byte < PACKET_KEY_BYTE_SIZE - 1; ++byte)
        {
            const auto &pair {orderedCorr[byte][value]};
            output += std::format("{}, {}, ", pair.first, static_cast<unsigned>(pair.second));

            if (input.victimKeyKnown and input.victimKey[byte] == pair.second)
                knownKeyCorrStats.insert(value);
        }
        const auto &pair {orderedCorr[PACKET_KEY_BYTE_SIZE - 1][value]};
        output += std::format("{}, {}\n", pair.first, static_cast<unsigned>(pair.second));
        if (input.victimKeyKnown and input.victimKey[PACKET_KEY_BYTE_SIZE - 1] == pair.second)
            knownKeyCorrStats.insert(value);
    }

    // if we have the key, we can print some additional (and useful) statistics.
    if (input.victimKeyKnown)
    {
        output += '\n';
        output += "Known key stats:\n"; //
        // Again, printing a new header.
        for (unsigned byte {0}; byte < PACKET_KEY_BYTE_SIZE - 1; ++byte)
        {
            output += std::format("KeyByte_{}_Pos, KeyByte_{}_Value", byte, byte);
        }
        output +=
            std::format("KeyByte_{}_Pos\n", PACKET_KEY_BYTE_SIZE - 1, PACKET_KEY_BYTE_SIZE - 1);

        // print the position of the key bytes in the ordered correlation array.
        for (unsigned byte {0}; byte < PACKET_KEY_BYTE_SIZE - 1; ++byte)
            output += std::format("{}, {}, ", knownKeyCorrStats[byte],
                                  static_cast<unsigned>(input.victimKey[byte]));
        output += std::format("{}, {}\n", knownKeyCorrStats[PACKET_KEY_BYTE_SIZE - 1],
                              static_cast<unsigned>(input.victimKey[PACKET_KEY_BYTE_SIZE - 1]));

        const auto &stats {knownKeyCorrStats.globalMetric()};
        output += std::format("Avg Pos: {}\nStd Dev: {}\nMax Pos: {}\nMin Pos: {}\n", stats.mean,
                              stats.stdDev, stats.max, stats.min);
    }

    std::cout << output;
    std::filesystem::create_directories(input.savePath);
    std::ofstream out {input.savePath};
    out << output;
    std::cout << "Finished Correlate job.\n\n";
}

std::string App::JobCorrelate::description() const noexcept
{
    std::string description {"Correlate - Victim Key: "};
    if (input.victimKeyKnown)
        for (std::byte byte : input.victimKey)
            description += std::to_string(static_cast<unsigned>(byte)) + ' ';
    else
        description += "Unknown ";

    description += "Victim Paths: {";
    for (const std::filesystem::path &path : input.victimLoadPaths)
        description += path.string() + ", ";
    description.pop_back(); // remove ' '
    description.pop_back(); // remove ','
    description += "} ";

    description += "Doppel Paths: {";
    for (const std::filesystem::path &path : input.doppelLoadPaths)
        description += path.string() + ", ";
    description.pop_back(); // remove ' '
    description.pop_back(); // remove ','
    description += '}';

    return description;
}

std::unique_ptr<App::JobI> App::JobCorrelate::clone() const
{
    return std::make_unique<JobCorrelate>(*this);
}
