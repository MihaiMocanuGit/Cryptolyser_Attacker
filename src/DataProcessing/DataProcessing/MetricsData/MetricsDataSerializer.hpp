#ifndef CRYPTOLYSER_ATTACKER_METRICSDATASERIALIZER_HPP
#define CRYPTOLYSER_ATTACKER_METRICSDATASERIALIZER_HPP

#include "../Serializer/Serializer.hpp"
#include "MetricsData.hpp"

#include <filesystem>
#include <format>
#include <fstream>
#include <string_view>

namespace Serializer
{
template <Real T>
void saveToCsv(const std::filesystem::path &path, const MetricsData<T> &metricsData)
{
    saveToCsv(path, metricsData.globalMetrics());
}

template <Real T>
void loadFromCsv(const std::filesystem::path &path, MetricsData<T> &metricsData)
{
    Metrics<T> metrics{};
    loadFromCsv(path, metrics);
    metricsData = MetricsData<T>{metrics};
}

} // namespace Serializer

#endif // CRYPTOLYSER_ATTACKER_METRICSDATASERIALIZER_HPP
