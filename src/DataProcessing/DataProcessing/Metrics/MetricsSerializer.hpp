#ifndef CRYPTOLYSER_ATTACKER_METRICSSERIALIZER_HPP
#define CRYPTOLYSER_ATTACKER_METRICSSERIALIZER_HPP

#include "Metrics.hpp"

#include <filesystem>
#include <format>
#include <fstream>

namespace Serializer
{
template <Real T>
void saveToCsv(const std::filesystem::path &path, const Metrics<T> &metrics)
{
    constexpr std::string_view header{"Sum,Size,Mean,Variance,StdDev,Min,Max\n"};
    std::filesystem::path directory = path;
    directory.remove_filename();
    std::filesystem::create_directories(directory);
    std::filesystem::path formattedPath{path};
    if (not path.has_extension())
        formattedPath.replace_extension("csv");
    std::ofstream out;
    out.open(formattedPath);

    if (!out)
        throw std::runtime_error("Saving Metrics | Could not create file: " +
                                 formattedPath.string());
    out << header;
    out << std::format("{},", metrics.sum)      // Sum
        << std::format("{},", metrics.size)     // Size
        << std::format("{},", metrics.mean)     // Mean
        << std::format("{},", metrics.variance) // Variance
        << std::format("{},", metrics.stdDev)   // StdDev
        << std::format("{},", metrics.min)      // Min
        << std::format("{}", metrics.max)       // Max
        << '\n';
    out.close();
}

template <Real T>
void loadFromCsv(const std::filesystem::path &path, Metrics<T> &metrics)
{
    std::filesystem::path formattedPath{path};
    if (not path.has_extension())
        formattedPath.replace_extension("csv");
    std::ifstream in;
    in.open(formattedPath);
    if (!in)
        throw std::runtime_error("Loading Metrics | Could not open file: " +
                                 formattedPath.string());

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
    metrics = {sum, size, mean, variance, stdDev, min, max};
    in.close();
}
} // namespace Serializer

#endif // CRYPTOLYSER_ATTACKER_METRICSSERIALIZER_HPP
