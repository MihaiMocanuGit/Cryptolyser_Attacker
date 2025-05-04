#include "JobCorrelate.hpp"

#include "Correlate/Correlate.hpp"
#include "Cryptolyser_Common/connection_data_types.h"
#include "DataProcessing/MetricsData/MetricsData.hpp"
#include "Study/SerializerManager/SerializerManager.hpp"
#include "Study/Study.hpp"
#include "Study/TimingData/TimingData.hpp"

std::string App::JobCorrelate::m_createCorrelationDataString(
    const Correlate<MetricsData<double>, MetricsData<double>> &correlate,
    SampleData<double> &byteCorrPos) const
{
    std::string dataString {""};
    const auto orderedCorr {correlate.order()};
    // Write header.
    for (unsigned byte {0}; byte < PACKET_KEY_BYTE_SIZE - 1; ++byte)
        dataString += std::format("Byte_{}_Corr, Byte_{}_Key, ", byte, byte);
    dataString +=
        std::format("Byte_{}_Corr, Byte_{}_Key\n", PACKET_KEY_BYTE_SIZE - 1,
                    PACKET_KEY_BYTE_SIZE - 1); // Write the pair of (correlation value , key value)
    for (unsigned value {0}; value < 256; value++)
    {
        for (unsigned byte {0}; byte < PACKET_KEY_BYTE_SIZE - 1; ++byte)
        {
            const auto &pair {orderedCorr[byte][value]};
            dataString += std::format("{}, {}, ", pair.first, static_cast<unsigned>(pair.second));

            if (this->input.victimKeyKnown and this->input.victimKey[byte] == pair.second)
                byteCorrPos.insert(value);
        }
        const auto &pair {orderedCorr[PACKET_KEY_BYTE_SIZE - 1][value]};
        dataString += std::format("{}, {}\n", pair.first, static_cast<unsigned>(pair.second));
        if (this->input.victimKeyKnown and
            this->input.victimKey[PACKET_KEY_BYTE_SIZE - 1] == pair.second)
            byteCorrPos.insert(value);
    }
    return dataString;
}

Correlate<MetricsData<double>, MetricsData<double>> App::JobCorrelate::m_computeCorrelation() const
{
    // Load all the known victim timing data
    std::vector<TimingData<false, MetricsData<double>>> victimData;
    victimData.reserve(this->input.victimLoadPaths.size());
    std::cout << "Loading the victim timing data...\n";
    for (const auto &loadPath : this->input.victimLoadPaths)
    {
        const auto metadata {SerializerManager::loadTimingMetadata(loadPath)};
        TimingData<false, MetricsData<double>> timingData {metadata.dataSize};
        SerializerManager::loadRaw(loadPath, timingData);

        victimData.emplace_back(std::move(timingData));
    }

    // Prepare an empty correlation
    Correlate<MetricsData<double>, MetricsData<double>> correlate {};

    // Load one by one the doppel timing data
    std::cout << "Loading the doppel timing data...\n";
    for (const auto &loadPath : this->input.doppelLoadPaths)
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
    return correlate;
}

std::string App::JobCorrelate::m_summariseKeyStats(
    const Correlate<MetricsData<double>, MetricsData<double>> &correlate,
    const SampleData<double> byteCorrPos) const
{
    std::string summary {
        ""}; // if we have the key, we can print some additional (and useful) statistics.
    if (input.victimKeyKnown)
    {
        summary += '\n';
        summary += "Stats for key:, ";
        for (std::byte keyByte : input.victimKey)
        {
            summary += std::to_string(static_cast<unsigned>(keyByte)) + ' ';
        }
        summary += '\n';
        // Printing a new header.
        summary += "Byte:, ";
        for (unsigned byte {0}; byte < PACKET_KEY_BYTE_SIZE - 1; ++byte)
        {
            summary += std::format("{}, ", byte);
        }
        summary += std::format("{}\n", PACKET_KEY_BYTE_SIZE - 1);

        // print the position of the key bytes in the ordered correlation array.
        summary += "Pos:, ";
        for (unsigned byte {0}; byte < PACKET_KEY_BYTE_SIZE - 1; ++byte)
            summary += std::format("{}, ", byteCorrPos[byte]);
        summary += std::format("{}\n", byteCorrPos[PACKET_KEY_BYTE_SIZE - 1]);

        // print the correlation value of the key bytes in the ordered correlation array.
        summary += "Corr:, ";
        for (unsigned byte {0}; byte < PACKET_KEY_BYTE_SIZE - 1; ++byte)
            summary += std::format(
                "{}, ", correlate.data()[byte][static_cast<unsigned>(input.victimKey[byte])]);
        summary +=
            std::format("{}\n", correlate.data()[PACKET_KEY_BYTE_SIZE - 1][static_cast<unsigned>(
                                    input.victimKey[PACKET_KEY_BYTE_SIZE - 1])]);

        // Print some metrics for the positions of key bytes.
        const auto &stats {byteCorrPos.globalMetric()};
        summary += std::format("\nAvg Pos:, {}\nStd Dev:, {}\nMax Pos:, {}\nMin Pos:, {}\n",
                               stats.mean, stats.stdDev, stats.max, stats.min);
    }
    return summary;
}

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
    auto correlate = m_computeCorrelation();

    SampleData<double> byteCorrPos {};
    std::string csvOutput {m_createCorrelationDataString(correlate, byteCorrPos)};

    std::string summary {m_summariseKeyStats(correlate, byteCorrPos)};
    csvOutput += summary;

    std::filesystem::create_directories(input.savePath);
    input.savePath /= "Correlation.csv";
    std::ofstream out {input.savePath};
    out << csvOutput;

    std::cout << summary;
    std::cout << "Check full correlation file at: " << input.savePath.string() << "\n\n";
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
