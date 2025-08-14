#ifndef CRYPTOLYSER_ATTACKER_SERIALIZERMANAGER_HPP
#define CRYPTOLYSER_ATTACKER_SERIALIZERMANAGER_HPP

#include "Cryptolyser_Common/connection_data_types.h"
#include "DataProcessing/DistributionData/DistributionDataSerializer.hpp"
#include "Study/TimingData/TimingData.hpp"

#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <unordered_map>

class SerializerManager
{
  public:
    static void saveDistribution(const std::filesystem::path &path,
                                 const DistributionData<double> &distributionData);

    template <bool KnownKey, HasMetric DataType = SampleData<double>>
    static void saveRaw(const std::filesystem::path &path,
                        const TimingData<KnownKey, DataType> &timingData);

    template <bool KnownKey, HasMetric DataType = SampleData<double>>
    static void loadRaw(const std::filesystem::path &path,
                        TimingData<KnownKey, DataType> &timingData);

    struct TimingMetadata
    {
        unsigned dataSize;
        bool knownKey;
        std::array<std::byte, PACKET_KEY_SIZE> key;
    };

    static TimingMetadata loadTimingMetadata(const std::filesystem::path &path);

    template <bool KnownKey, HasMetric DataType = SampleData<double>>
    static void saveMetrics(const std::filesystem::path &path,
                            const TimingData<KnownKey, DataType> &data);

    template <HasMetric DataType>
    static void saveRaw(const std::filesystem::path &path, const DataType &timingData);
};

template <bool KnownKey, HasMetric DataType>
void SerializerManager::saveRaw(const std::filesystem::path &path,
                                const TimingData<KnownKey, DataType> &timingData)
{
    // Save the metadata to a new file in the same directory.
    std::filesystem::path metadata {path / "meta.data"};
    std::filesystem::create_directories(path);
    std::ofstream out {metadata};
    out << "$DataSize: " << timingData.dataSize() << ";\n";
    out << "$KnownKey: " << KnownKey << ";\n";
    if constexpr (KnownKey)
    {
        std::string keyStr {""};
        for (std::byte byte : timingData.key())
            keyStr += std::format("{:02x} ", static_cast<unsigned>(byte));
        keyStr.pop_back(); // delete the last ' '

        out << "$Key: " << keyStr << ";\n";
    }
    out.close();

    Serializer::saveToCsv(path, timingData.timing());
}

template <bool KnownKey, HasMetric DataType>
void SerializerManager::loadRaw(const std::filesystem::path &path,
                                TimingData<KnownKey, DataType> &timingData)
{
    if (std::filesystem::exists(path / "meta.data"))
        Serializer::loadFromCsv(path, timingData.timing());
    else if (std::filesystem::exists(path / "Raw" / "meta.data"))
        Serializer::loadFromCsv(path / "Raw", timingData.timing());
    else
        throw std::runtime_error {std::format("No timing data to load here: {}", path.string())};
}

template <bool KnownKey, HasMetric DataType>
void SerializerManager::saveMetrics(const std::filesystem::path &path,
                                    const TimingData<KnownKey, DataType> &timingData)
{
    std::filesystem::create_directories(path);
    for (unsigned byte {0}; byte < PACKET_AES_BLOCK_SIZE; ++byte)
    {
        constexpr std::string_view header {"Value,Size,Mean,StdDev,MeanStd,StdDevStd,Min,Max\n"};
        const std::filesystem::path filePath {path / ("Byte_" + std::to_string(byte) + ".csv")};
        std::ofstream out;
        out.open(filePath);
        if (!out)
            throw std::runtime_error("Saving Metrics | Could not create file: " +
                                     filePath.string());
        out << header;
        for (unsigned byteValue {0}; byteValue < 256; ++byteValue)
        {
            const auto &metric = timingData[byte][byteValue].globalMetric();
            const auto &stdMetric = timingData[byte].standardizeMetric(byteValue);
            out << std::format("{},", byteValue)        // ByteValue
                << std::format("{},", metric.size)      // Size
                << std::format("{},", metric.mean)      // Mean
                << std::format("{},", metric.stdDev)    // StdDev
                << std::format("{},", stdMetric.mean)   // MeanStd
                << std::format("{},", stdMetric.stdDev) // StdDevStd
                << std::format("{},", metric.min)       // Min
                << std::format("{}", metric.max)        // Max
                << '\n';
        }
        out.close();
    }
}

template <HasMetric DataType>
void SerializerManager::saveRaw(const std::filesystem::path &path, const DataType &data)
{
    Serializer::saveToCsv(path, data);
}
#endif // CRYPTOLYSER_ATTACKER_SERIALIZERMANAGER_HPP
