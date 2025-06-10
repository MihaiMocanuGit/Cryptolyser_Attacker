#include "SerializerManager.hpp"

#include <filesystem>
#include <stdexcept>

void SerializerManager::saveDistribution(const std::filesystem::path &path,
                                         const DistributionData<double> &distributionData)
{
    Serializer::saveToCsv(path, distributionData);
}

SerializerManager::TimingMetadata
    SerializerManager::loadTimingMetadata(const std::filesystem::path &path)
{
    std::filesystem::path metadataPath {};
    if (std::filesystem::exists(path / "meta.data"))
        metadataPath = path / "meta.data";
    else if (std::filesystem::exists(path / "Raw" / "meta.data"))
        metadataPath = path / "Raw" / "meta.data";
    else
        throw std::runtime_error(std::format("No meta.data to load here: {}", path.string()));

    std::ifstream in {metadataPath};
    std::string metadata {""};
    std::string line;
    while (std::getline(in, line))
        metadata += line;
    in.close();

    TimingMetadata result {};
    using namespace std::string_literals;
    // DATA SIZE
    size_t start {metadata.find("$DataSize:") + "$DataSize:"s.size()};
    while (metadata[start] == ' ')
        start++;
    size_t end {metadata.find(';', start)};
    std::string dataSizeStr {metadata.substr(start, end - start)};
    result.dataSize = static_cast<unsigned>(std::stoul(dataSizeStr));

    // IV
    start = metadata.find("$IV:", end) + "$IV:"s.size();
    while (metadata[start] == ' ')
        start++;
    end = metadata.find(";", start);
    std::string ivStr {metadata.substr(start, end - start)};
    std::stringstream ivStream {ivStr};
    for (std::byte &byte : result.IV)
    {
        unsigned byteTmp;
        ivStream >> std::hex >> byteTmp;
        byte = static_cast<std::byte>(byteTmp);
    }

    // KNOWN KEY
    start = metadata.find("$KnownKey:", end) + "$KnownKey:"s.size();
    while (metadata[start] == ' ')
        start++;
    end = metadata.find(";", start);
    std::string knownKeyStr {metadata.substr(start, end - start)};
    result.knownKey = std::stoul(knownKeyStr) != 0;

    if (result.knownKey)
    {
        // KEY
        start = metadata.find("$Key:", end) + "$Key:"s.size();
        while (metadata[start] == ' ')
            start++;
        end = metadata.find(";", start);
        std::string keyStr {metadata.substr(start, end - start)};
        std::stringstream keyStream {keyStr};
        for (std::byte &byte : result.key)
        {
            unsigned byteTmp;
            keyStream >> std::hex >> byteTmp;
            byte = static_cast<std::byte>(byteTmp);
        }
    }

    return result;
}
