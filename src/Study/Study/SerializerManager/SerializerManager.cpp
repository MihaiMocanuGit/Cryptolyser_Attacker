#include "SerializerManager.hpp"

void SerializerManager::saveDistribution(const std::filesystem::path &path,
                                         const DistributionData<double> &distributionData)
{
    Serializer::saveToCsv(path, distributionData);
}

SerializerManager::TimingMetadata
    SerializerManager::loadTimingMetadata(const std::filesystem::path &path)
{
    std::unordered_map<std::string_view, std::string> metadataVars;
    std::filesystem::path metadataPath {path / "meta.data"};
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
