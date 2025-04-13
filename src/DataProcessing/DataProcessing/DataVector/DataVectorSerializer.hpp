#ifndef CRYPTOLYSER_ATTACKER_DATAVECTORSERIALIZER_HPP
#define CRYPTOLYSER_ATTACKER_DATAVECTORSERIALIZER_HPP

#include "../Serializer/Serializer.hpp"
#include "DataVector.hpp"

#include <filesystem>
#include <format>
#include <fstream>
#include <string_view>

namespace Serializer
{
/// TODO: Thread smart allocation: Create a system that does limit the max  no of threads used
///  inside the "Recursive" function calls. What if instead of this Serializer Namespace we used a
///  class with static private and public functions/variables? Would this work? Are namespaces and
///  static class functions in the same scope, if they share the same name?
template <HasMetric T>
void saveToCsv(const std::filesystem::path &path, const DataVector<T> &metricsVector)
{
    std::filesystem::create_directories(path);

    unsigned numberThreads{std::thread::hardware_concurrency() / 2};
    if (not numberThreads)
        numberThreads = 1;

    std::vector<std::thread> threads;
    threads.reserve(numberThreads);
    const size_t FILES_PER_THREAD{metricsVector.size() / numberThreads};
    const size_t REMAINING_FILES{metricsVector.size() % numberThreads};

    auto threadBlock = [&](size_t threadNo, size_t filesNo, size_t remainingFiles = 0)
    {
        for (size_t file{0}; file < filesNo + remainingFiles; ++file)
        {
            const size_t IdValue = file + threadNo * filesNo;
            const std::filesystem::path nextPath{path / std::to_string(IdValue)};
            // Note that metricsVector[IdValue] could be a file or another vector (aka another
            // directory).
            saveToCsv(nextPath, metricsVector[IdValue]);
        }
    };

    for (unsigned thread{0}; thread < numberThreads - 1; thread++)
        threads.emplace_back(threadBlock, thread, FILES_PER_THREAD);
    // The last thread will also be responsible for the remaining number files.
    threadBlock(numberThreads - 1, FILES_PER_THREAD, REMAINING_FILES);

    for (unsigned thread{0}; thread < numberThreads - 1; ++thread)
        threads[thread].join();
}

template <HasMetric T>
void loadFromCsv(const std::filesystem::path &path, DataVector<T> &metricsVector)
{
    unsigned numberThreads = std::thread::hardware_concurrency() / 2;
    if (not numberThreads)
        numberThreads = 1;
    std::vector<std::thread> threads;
    threads.reserve(numberThreads);

    // Find out how many directories or files we need to read.
    size_t size{0};
    while (std::filesystem::exists(path / (std::to_string(size) + ".csv")) or
           std::filesystem::exists(path / std::to_string(size)))
        size++;

    const size_t FILES_PER_THREAD{size / numberThreads};
    const size_t REMAINING_FILES{size % numberThreads};

    // Prepare enough space for the vector to hold the identified number of files or directories.
    for (size_t currentCount{metricsVector.size()}; currentCount < size; currentCount++)
        metricsVector.add({});

    std::mutex mutex;
    auto threadBlock = [&](size_t threadNo, size_t filesNo, size_t remainingFiles = 0)
    {
        for (size_t file{0}; file < filesNo + remainingFiles; ++file)
        {
            const size_t idValue{file + threadNo * filesNo};
            const std::filesystem::path nextPath{path / std::to_string(idValue)};
            // Note that T in the current context could be either SampleData<double> or
            // DataVector<SampleData<double>>
            T nextLoad{};
            loadFromCsv(nextPath, nextLoad);
            // A lock is necessary as inserting modifies the global metrics
            const std::lock_guard<std::mutex> lockGuard{mutex};
            // in this case we want to join every element from nextLoad with every element
            // already present in the vector held at index idValue
            metricsVector.update(idValue,
                                 [&nextLoad](T &elem) { joinDataVectors(elem, nextLoad); });
        }
    };

    for (unsigned thread{0}; thread < numberThreads - 1; thread++)
        threads.emplace_back(threadBlock, thread, FILES_PER_THREAD);
    // The last thread will also be responsible for the remaining number of files.
    threadBlock(numberThreads - 1, FILES_PER_THREAD, REMAINING_FILES);

    for (unsigned thread{0}; thread < numberThreads - 1; ++thread)
        threads[thread].join();
}
} // namespace Serializer
#endif // CRYPTOLYSER_ATTACKER_DATAVECTORSERIALIZER_HPP
