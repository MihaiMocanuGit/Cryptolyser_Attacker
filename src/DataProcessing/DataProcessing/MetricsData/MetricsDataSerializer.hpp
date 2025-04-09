#ifndef CRYPTOLYSER_ATTACKER_METRICSDATASERIALIZER_HPP
#define CRYPTOLYSER_ATTACKER_METRICSDATASERIALIZER_HPP

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
    constexpr std::string_view header{"Sum,Size,Mean,Variance,StdDev,Min,Max\n"};
    std::filesystem::path directory = path;
    directory.remove_filename();
    std::filesystem::create_directories(directory);
    std::ofstream out;
    out.open(path);
    if (!out)
        throw std::runtime_error("Saving MetricData | Could not create file: " + path.string());
    out << header;
    out << std::format("{},", metricsData.globalMetric().sum)      // Sum
        << std::format("{},", metricsData.globalMetric().size)     // Size
        << std::format("{},", metricsData.globalMetric().mean)     // Mean
        << std::format("{},", metricsData.globalMetric().variance) // Variance
        << std::format("{},", metricsData.globalMetric().stdDev)   // StdDev
        << std::format("{},", metricsData.globalMetric().min)      // Min
        << std::format("{}", metricsData.globalMetric().max)       // Max
        << '\n';
    out.close();
}

template <Real T>
void loadFromCsv(const std::filesystem::path &path, MetricsData<T> &metricsData)
{
    std::ifstream in;
    in.open(path);
    if (!in)
        throw std::runtime_error("Loading MetricData | Could not create file: " + path.string());
    std::string header;
    std::getline(in, header);
    T sum, mean, variance, stdDev, min, max;
    size_t size;
    char comma;
    in >> sum >> comma       // Sum
        >> size >> comma     // Size
        >> mean >> comma     // Mean
        >> variance >> comma // Variance
        >> stdDev >> comma   // StdDev
        >> min >> comma      // Min
        >> max;              // Max
    SampleMetrics<T> metrics{sum, size, mean, variance, stdDev, min, max};
    metricsData = MetricsData<T>{metrics};
}

} // namespace Serializer

#endif // CRYPTOLYSER_ATTACKER_METRICSDATASERIALIZER_HPP
