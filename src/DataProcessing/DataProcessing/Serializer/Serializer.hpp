#ifndef CRYPTOLYSER_ATTACKER_SERIALIZER_HPP
#define CRYPTOLYSER_ATTACKER_SERIALIZER_HPP

#include "../DataVector/DataVector.hpp"
#include "../DataVector/DataVectorSerializer.hpp"
#include "../MetricsData/MetricsData.hpp"
#include "../MetricsData/MetricsDataSerializer.hpp"
#include "../SampleData/SampleData.hpp"
#include "../SampleData/SampleDataSerializer.hpp"
#include "../Samples/SampleMetrics.hpp"

#include <filesystem>

namespace Serializer
{
template <Real T>
extern void saveToCsv(const std::filesystem::path &path, const MetricsData<T> &metricsData);

template <Real T>
extern void loadFromCsv(const std::filesystem::path &path, MetricsData<T> &metricsData);

template <Real T>
extern void saveToCsv(const std::filesystem::path &path, const New::SampleData<T> &sampleData);

template <Real T>
extern void loadFromCsv(const std::filesystem::path &path, New::SampleData<T> &sampleData);

template <HasMetric T>
extern void saveToCsv(const std::filesystem::path &path, const DataVector<T> &metricsVector);

template <HasMetric T>
extern void loadFromCsv(const std::filesystem::path &path, DataVector<T> &metricsVector);

} // namespace Serializer
#endif // CRYPTOLYSER_ATTACKER_SERIALIZER_HPP
