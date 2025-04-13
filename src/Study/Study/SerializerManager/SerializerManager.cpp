#include "SerializerManager.hpp"

void SerializerManager::saveDistribution(const std::filesystem::path &path,
                                         const DistributionData<double> &distributionData)
{
    Serializer::saveToCsv(path, distributionData);
}
