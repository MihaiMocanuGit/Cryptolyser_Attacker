#include "JobCorrelate.hpp"

#include "Correlate/Correlate.hpp"
#include "Cryptolyser_Common/connection_data_types.h"
#include "DataProcessing/MetricsData/MetricsData.hpp"
#include "DataProcessing/SampleData/SampleData.hpp"
#include "Study/SerializerManager/SerializerManager.hpp"
#include "Study/TimingData/TimingData.hpp"

#include <iostream>

namespace App
{
namespace
{
void repairLineEnd(std::string &line)
{
    if (line.ends_with(' '))
        line.pop_back();
    if (line.ends_with(','))
        line.pop_back();
    line += '\n';
}
} // namespace

std::string JobCorrelate::m_createCorrelationDataString(
    const Correlate<MetricsData<double>, MetricsData<double>> &correlate,
    std::array<unsigned, PACKET_AES_BLOCK_SIZE> &byteCorrPos) const
{
    std::string dataString {""};
    const auto orderedCorr {correlate.order()};

    // Write header.
    for (unsigned byte {0}; byte < PACKET_KEY_SIZE; ++byte)
        dataString += std::format("Byte_{}_Corr, Byte_{}_Key, ", byte, byte);
    repairLineEnd(dataString);

    // Write the Corr - Key pair for every value (in decreasing order relative to Corr)
    for (unsigned rank {0}; rank < 256; rank++)
    {
        for (unsigned byte {0}; byte < PACKET_KEY_SIZE; ++byte)
        {
            const auto &pair {orderedCorr[byte][rank]};
            dataString +=
                std::format("{}, {:#04x}, ", pair.first, static_cast<unsigned>(pair.second));

            if (this->input.victimKeyKnown and this->input.victimKey[byte] == pair.second)
                byteCorrPos[byte] = rank;
        }
        repairLineEnd(dataString);
    }
    return dataString;
}

Correlate<MetricsData<double>, MetricsData<double>> JobCorrelate::m_computeCorrelation() const
{
    auto processGroup = [&](const Input::Group &group, unsigned groupId)
    {
        std::cout << "Group " << groupId << "\n";
        // Load all the known victim timing data
        std::vector<TimingData<false, MetricsData<double>>> victimData;
        victimData.reserve(group.victimLoadPaths.size());
        std::cout << "Loading the victim timing data...\n";
        for (const auto &loadPath : group.victimLoadPaths)
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
        for (const auto &loadPath : group.doppelLoadPaths)
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
    };

    Correlate<MetricsData<double>, MetricsData<double>> correlate {};
    unsigned groupId {0};
    for (const auto &group : input.groups)
    {
        correlate += processGroup(group, groupId);
        groupId++;
    }

    return correlate;
}

std::string JobCorrelate::m_summariseKeyStats(
    const Correlate<MetricsData<double>, MetricsData<double>> &correlate,
    const std::array<unsigned, PACKET_AES_BLOCK_SIZE> &byteCorrPos) const
{
    std::string summary {""};
    // if we have the key, we can print some additional (and useful) statistics.
    if (input.victimKeyKnown)
    {
        summary += '\n';
        summary += "Stats for key:, ";
        for (std::byte keyByte : input.victimKey)
            summary += std::format("{:02x}, ", static_cast<unsigned>(keyByte));
        repairLineEnd(summary);

        // Printing a new header.
        summary += "Byte:, ";
        for (unsigned byte {0}; byte < PACKET_KEY_SIZE; ++byte)
            summary += std::format("{}, ", byte);
        repairLineEnd(summary);

        // print the position of the key bytes in the ordered correlation array.
        summary += "Pos:, ";
        for (unsigned byte {0}; byte < PACKET_KEY_SIZE; ++byte)
            summary += std::format("{}, ", byteCorrPos[byte]);
        repairLineEnd(summary);

        // print the correlation value of the key bytes in the ordered correlation array.
        summary += "Corr:, ";
        for (unsigned byte {0}; byte < PACKET_KEY_SIZE; ++byte)
            summary += std::format(
                "{}, ", correlate.data()[byte][static_cast<unsigned>(input.victimKey[byte])]);
        repairLineEnd(summary);

        // Print some metrics for the positions of key bytes.
        auto stats {SampleData<double> {byteCorrPos.begin(), byteCorrPos.end()}.globalMetric()};
        summary += std::format("\nAvg Pos:, {}\nStd Dev:, {}\nMax Pos:, {}\nMin Pos:, {}\n",
                               stats.mean, stats.stdDev, stats.max, stats.min);
    }
    return summary;
}

JobCorrelate::JobCorrelate(const JobCorrelate::Buffers &buffers) : JobI {}
{
    input.savePath = buffers.savePath;
    input.victimKeyKnown = buffers.victimKeyKnown;
    input.victimKey = buffers.victimKey;

    for (const auto &bufferGroup : buffers.groups)
    {
        input.groups.emplace_back();
        auto &inputGroup = input.groups.back();
        for (const auto &path : bufferGroup.victimLoadPaths)
            inputGroup.victimLoadPaths.emplace_back(path);
        for (const auto &path : bufferGroup.doppelLoadPaths)
            inputGroup.doppelLoadPaths.emplace_back(path);
    }
}

[[nodiscard]] JobCorrelate::ExitStatus_e JobCorrelate::invoke(std::stop_token stoken)
{
    if (stoken.stop_requested())
        return ExitStatus_e::KILLED;
    std::cout << "Started Correlate job...\n";
    auto correlate = m_computeCorrelation();

    if (stoken.stop_requested())
        return ExitStatus_e::KILLED;
    std::array<unsigned, PACKET_AES_BLOCK_SIZE> byteCorrPos {};
    std::string csvOutput {m_createCorrelationDataString(correlate, byteCorrPos)};

    if (stoken.stop_requested())
        return ExitStatus_e::KILLED;
    std::string summary {m_summariseKeyStats(correlate, byteCorrPos)};
    csvOutput += summary;

    if (stoken.stop_requested())
        return ExitStatus_e::KILLED;
    std::filesystem::create_directories(input.savePath);
    input.savePath /= "Correlation.csv";

    std::ofstream out {input.savePath};
    if (stoken.stop_requested())
        return ExitStatus_e::KILLED;
    out << csvOutput;

    std::cout << summary;
    std::cout << "Check full correlation file at: " << input.savePath.string() << "\n\n";
    std::cout << "Finished Correlate job.\n\n";
    return ExitStatus_e::OK;
}

std::string JobCorrelate::description() const noexcept
{
    std::string description {"Correlate - Victim Key: "};
    if (input.victimKeyKnown)
    {
        for (std::byte byte : input.victimKey)
            description += std::format("{:02x} ", static_cast<unsigned>(byte));
        description.pop_back();
        description += ", ";
    }
    else
        description += "Unknown ";

    unsigned groupId {0};
    for (const auto &group : input.groups)
    {
        description += "Group " + std::to_string(groupId) + ": {";
        description += "Victim Paths: {";
        for (const std::filesystem::path &path : group.victimLoadPaths)
            description += path.string() + ", ";
        description.pop_back(); // remove ' '
        description.pop_back(); // remove ','
        description += "} ";

        description += "Doppel Paths: {";
        for (const std::filesystem::path &path : group.doppelLoadPaths)
            description += path.string() + ", ";
        description.pop_back(); // remove ' '
        description.pop_back(); // remove ','
        description += '}';
        description += "} ";

        groupId++;
    }

    return description;
}

std::unique_ptr<JobI> JobCorrelate::clone() const { return std::make_unique<JobCorrelate>(*this); }

} // namespace App
