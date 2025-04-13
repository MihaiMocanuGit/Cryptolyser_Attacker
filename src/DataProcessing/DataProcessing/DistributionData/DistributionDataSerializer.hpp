#ifndef CRYPTOLYSER_ATTACKER_DISTRIBUTIONDATASERIALIZER_HPP
#define CRYPTOLYSER_ATTACKER_DISTRIBUTIONDATASERIALIZER_HPP

#include "../Serializer/Serializer.hpp"
#include "DistributionData.hpp"

namespace Serializer
{

template <Real T>
void saveToCsv(const std::filesystem::path &path, const DistributionData<T> &distributionData)
{
    std::filesystem::path directory = path;
    directory.remove_filename();
    std::filesystem::create_directories(directory);
    // Using this format in order to have the ability to call loadFromCsv(path, MetricsData) on this
    // file.
    std::filesystem::path formattedPath{path};
    if (not path.has_extension())
        formattedPath.replace_extension("csv");
    std::ofstream out;
    out.open(formattedPath);
    if (!out)
        throw std::runtime_error("Saving Sample Data | Could not create file: " +
                                 formattedPath.string());
    const auto metric{distributionData.globalMetric()};
    constexpr std::string_view headerMetrics{"Sum,Size,Mean,Variance,StdDev,Min,Max\n"};
    out << headerMetrics;
    out << std::format("{},", metric.sum)      // Sum
        << std::format("{},", metric.size)     // Size
        << std::format("{},", metric.mean)     // Mean
        << std::format("{},", metric.variance) // Variance
        << std::format("{},", metric.stdDev)   // StdDev
        << std::format("{},", metric.min)      // Min
        << std::format("{}", metric.max);      // Max
    out << '\n';
    constexpr std::string_view headerValues{"Index,Values\n"};
    out << headerValues;
    for (size_t pos{0}; pos < distributionData.size(); ++pos)
        out << distributionData.computeGlobalIndex(pos) << ','
            << std::format("{}", distributionData[pos]) << '\n';
}

template <Real T>
void loadFromCsv(const std::filesystem::path &path, DistributionData<T> &distributionData)
{
    std::filesystem::path formattedPath{path};
    if (not path.has_extension())
        formattedPath.replace_extension("csv");
    std::ifstream in;
    in.open(formattedPath);
    if (!in)
        throw std::runtime_error("Loading SampleData | Could not open file: " +
                                 formattedPath.string());
    std::string header;
    std::getline(in, header);
    T frequency, sum, mean, variance, stdDev, min, max;
    size_t globalIndex, size;
    char comma;
    in >> sum >> comma       // Sum
        >> size >> comma     // Size
        >> mean >> comma     // Mean
        >> variance >> comma // Variance
        >> stdDev >> comma   // StdDev
        >> min >> comma      // Min
        >> max;              // Max
    in >> std::ws;
    // Removing the second header: Index, Value
    std::getline(in, header);
    for (size_t i{0}; i < size; ++i)
    {
        in >> globalIndex >> comma >> frequency;
        distributionData.update(distributionData.computeLocalIndex(globalIndex), frequency);
    }
}

} // namespace Serializer
#endif // CRYPTOLYSER_ATTACKER_DISTRIBUTIONDATASERIALIZER_HPP
