#ifndef CRYPTOLYSER_ATTACKER_SAMPLEDATASERIALIZER_HPP
#define CRYPTOLYSER_ATTACKER_SAMPLEDATASERIALIZER_HPP

#include "../Serializer/Serializer.hpp"
#include "SampleData.hpp"

#include <filesystem>
#include <format>
#include <fstream>
#include <string_view>

namespace Serializer
{
template <Real T>
void saveToCsv(const std::filesystem::path &path, const SampleData<T> &sampleData)
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
    constexpr std::string_view headerMetrics{"Sum,Size,Mean,Variance,StdDev,Min,Max\n"};
    out << headerMetrics;
    out << std::format("{},", sampleData.globalMetric().sum)      // Sum
        << std::format("{},", sampleData.globalMetric().size)     // Size
        << std::format("{},", sampleData.globalMetric().mean)     // Mean
        << std::format("{},", sampleData.globalMetric().variance) // Variance
        << std::format("{},", sampleData.globalMetric().stdDev)   // StdDev
        << std::format("{},", sampleData.globalMetric().min)      // Min
        << std::format("{}", sampleData.globalMetric().max);      // Max
    out << '\n';
    constexpr std::string_view headerValues{"Index,Values\n"};
    out << headerValues;
    for (size_t i{0}; i < sampleData.size(); ++i)
        out << i << ',' << std::format("{}", sampleData[i]) << '\n';
}

template <Real T>
void loadFromCsv(const std::filesystem::path &path, SampleData<T> &sampleData)
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
    T value, sum, mean, variance, stdDev, min, max;
    size_t index, size;
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
    sampleData.reserve(sampleData.size() + size);
    std::vector<T> sample{};
    sample.reserve(size);
    for (size_t i{0}; i < size; ++i)
    {
        in >> index >> comma >> value;
        sample.push_back(value);
    }
    // Inserting only one large chunk seems to be faster than making an insert for every line.
    sampleData.insert(sample);
}
} // namespace Serializer
#endif // CRYPTOLYSER_ATTACKER_SAMPLEDATASERIALIZER_HPP
