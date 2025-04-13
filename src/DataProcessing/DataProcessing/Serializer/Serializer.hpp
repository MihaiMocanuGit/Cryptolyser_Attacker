#ifndef CRYPTOLYSER_ATTACKER_SERIALIZER_HPP
#define CRYPTOLYSER_ATTACKER_SERIALIZER_HPP

#include "../DataVector/DataVector.hpp"
#include "../DataVector/DataVectorSerializer.hpp"
#include "../DistributionData/DistributionData.hpp"
#include "../Metrics/Metrics.hpp"
#include "../Metrics/MetricsSerializer.hpp"
#include "../MetricsData/MetricsData.hpp"
#include "../MetricsData/MetricsDataSerializer.hpp"
#include "../SampleData/SampleData.hpp"
#include "../SampleData/SampleDataSerializer.hpp"

#include <filesystem>

// While these function declarations are not needed for linkage/compiling, I've decided to include
// them to clarify the API structure.
namespace Serializer
{
template <Real T>
extern void saveToCsv(const std::filesystem::path &path, const Metrics<T> &metrics);

template <Real T>
extern void loadFromCsv(const std::filesystem::path &path, Metrics<T> &metrics);

template <Real T>
extern void saveToCsv(const std::filesystem::path &path, const MetricsData<T> &metricsData);

template <Real T>
extern void loadFromCsv(const std::filesystem::path &path, MetricsData<T> &metricsData);

template <Real T>
extern void saveToCsv(const std::filesystem::path &path, const New::SampleData<T> &sampleData);

template <Real T>
extern void loadFromCsv(const std::filesystem::path &path, New::SampleData<T> &sampleData);

template <Real T>
extern void saveToCsv(const std::filesystem::path &path,
                      const DistributionData<T> &distributionData);

template <Real T>
extern void loadFromCsv(const std::filesystem::path &path, DistributionData<T> &distributionData);

template <HasMetric T>
extern void saveToCsv(const std::filesystem::path &path, const DataVector<T> &metricsVector);

template <HasMetric T>
extern void loadFromCsv(const std::filesystem::path &path, DataVector<T> &metricsVector);

} // namespace Serializer
#endif // CRYPTOLYSER_ATTACKER_SERIALIZER_HPP
