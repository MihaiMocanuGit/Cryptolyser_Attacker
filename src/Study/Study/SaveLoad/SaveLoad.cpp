#include "SaveLoad.hpp"

#include "DataProcessing/Distribution/DistributionByteBlock.hpp"

#include <format>
#include <fstream>
#include <thread>

namespace SaveLoad
{

void saveDistributionByteValue(const std::filesystem::path &filename,
                               const DistributionByteValue &distribution)
{
    std::filesystem::path directory = filename;
    directory.remove_filename();
    std::filesystem::create_directories(directory);

    constexpr std::string_view CSV_HEADER{"Position, Frequency, Size, Start, Peak, Stop\n"};
    std::ofstream out;
    out.open(filename);
    if (!out)
        throw std::runtime_error("Saving Distribution Byte Value ERROR | Could not create file: " +
                                 filename.string());
    out << CSV_HEADER;

    size_t startPos{distribution.start()};
    size_t stopPos{distribution.stop()};

    // The distribution only holds frequencies starting from the first non-zero value. For usability
    // reasons, when saving to file, there will be appended at the beginning a count of startPos
    // zeros.
    // The first line is tricky, as it also holds other metrics. Moreover, the case when
    // startPos == 0 needs to be taken care of.
    out << '0' << ','                             // Position
        << ((startPos > 0) ? 0 : startPos) << ',' // Frequency
        << distribution.frequency().size() << ',' // Size
        << startPos << ','                        // Start
        << distribution.peakValue() << ','        // Peak
        << stopPos << '\n';                       // Stop

    // Append the necessary zeros
    for (size_t i{1}; i < startPos; ++i)
        out << i << ',' << 0 << '\n';

    for (size_t i{(startPos > 0) ? startPos : startPos + 1}; i <= stopPos; ++i)
        out << i << ',' << distribution.frequency()[i - startPos] << '\n';
}

void saveMetricsFromSampleGroup(const std::filesystem::path &filename,
                                const SampleGroup<double> &sampleGroup)
{
    std::filesystem::path directory = filename;
    directory.remove_filename();
    std::filesystem::create_directories(directory);

    DistributionByteBlock distributions{sampleGroup};

    constexpr std::string_view CSV_HEADER{
        "Value, Mean, StdDev, Size, StandardizedMean, StandardizedStdDev, Min, Max, PeakPos\n"};
    std::ofstream out;
    out.open(filename);
    if (!out)
        throw std::runtime_error(
            "Saving Metrics From Sample Group ERROR | Could not create file: " + filename.string());
    out << CSV_HEADER;
    for (unsigned byteValue = 0; byteValue < sampleGroup.size(); ++byteValue)
    {
        long double peakValue{0};
        long double weight{0};
        // Experimental metric used to create a statistic from the peak value, and it's right tail.
        // The motivation of the right tail is that perhaps this would better represent the cache
        // misses.
        size_t peakIndex = distributions.distributions()[byteValue].peakValue() -
                           distributions.distributions()[byteValue].start();
        for (unsigned tests{0}; tests < 16; ++tests)
        {
            size_t index{peakIndex + tests};
            size_t indexPos{index + distributions.distributions()[byteValue].start()};
            if (indexPos <= distributions.distributions()[byteValue].stop())
            {
                long double w{static_cast<long double>(
                    distributions.distributions()[byteValue].frequency()[index])};
                peakValue += w * index;
                weight += w;
            }
            else
                break;
        }
        peakValue /= weight;
        peakValue += distributions.distributions()[byteValue].start();

        SampleMetrics metrics = sampleGroup.localMetrics(byteValue);
        SampleMetrics standardizedMetrics = sampleGroup.standardizeLocalMetrics(byteValue);
        out << static_cast<int>(static_cast<uint8_t>(byteValue)) << ", " // Value
            << std::setprecision(8) << std::fixed                        //
            << metrics.mean << ", "                                      // Mean
            << metrics.stdDev << ", "                                    // StdDev
            << metrics.size << ", "                                      // Size
            << standardizedMetrics.mean << ", "                          // StdMean
            << standardizedMetrics.stdDev << ", "                        // StdDev
            << metrics.min << ", "                                       // Min
            << metrics.max << ", "                                       // Max
            << peakValue << "\n";                                        // Peak
    }
    out.close();
}

template <bool KnownKey>
void saveMetricsFromTimingData(const std::filesystem::path &directory,
                               const TimingData<KnownKey> &timingData)
{
    std::filesystem::create_directories(directory);
    constexpr unsigned NO_THREADS = 8;
    std::vector<std::thread> threads;
    threads.reserve(NO_THREADS);
    const unsigned FILES_PER_THREAD = timingData.blockTimings.size() / NO_THREADS;
    const unsigned REMAINING_FILES = timingData.blockTimings.size() % NO_THREADS;

    auto threadBlock = [&](unsigned threadNo, unsigned filesNo)
    {
        for (unsigned file{0}; file < filesNo; ++file)
        {
            const unsigned byteBlock = file + threadNo * filesNo;
            const std::filesystem::path filename =
                directory / ("Byte_" + std::to_string(byteBlock) + ".csv");
            saveMetricsFromSampleGroup(filename, timingData.blockTimings[byteBlock]);
        }
    };

    for (unsigned thread{0}; thread < NO_THREADS - 1; thread++)
        threads.emplace_back(threadBlock, thread, FILES_PER_THREAD);
    // The last thread will also be responsible for the remaining number files.
    threads.emplace_back(threadBlock, NO_THREADS - 1, FILES_PER_THREAD + REMAINING_FILES);

    for (unsigned thread{0}; thread < NO_THREADS; ++thread)
        threads[thread].join();
}

template <typename Real_t>
void saveRawFromSampleData(const std::filesystem::path &filename,
                           const SampleData<Real_t> &sampleData)
{
    std::filesystem::path directory = filename;
    directory.remove_filename();
    std::filesystem::create_directories(directory);

    constexpr std::string_view CSV_HEADER{"Index, Value, Mean, StdDev, Size, Min, Max\n"};
    std::ofstream out;
    out.open(filename);
    if (!out)
        throw std::runtime_error("Saving Raw From Sample Data ERROR | Could not create file: " +
                                 filename.string());
    out << CSV_HEADER;

    if (not sampleData.data().empty())
        out << "0," << std::format("{}", sampleData[0]) << ',';
    else
        out << ",,";
    out << sampleData.metrics().mean << ',' << sampleData.metrics().stdDev << ','
        << sampleData.metrics().size << ',' << sampleData.metrics().min << ','
        << sampleData.metrics().max << '\n';

    for (size_t i{1}; i < sampleData.size(); ++i)
        out << i << ',' << std::format("{}", sampleData[i]) << '\n';
}

void loadRawFromSampleData(const std::filesystem::path &filename, SampleData<double> &sampleData)
{
    std::ifstream in;
    in.open(filename);
    if (!in)
        throw std::runtime_error("Loading from SampleData ERROR | Could not open file: " +
                                 filename.string());
    // Example file:
    // Value, Mean, StdDev, Size, Min, Max
    // 0,972,975,32,7664,950,1000
    // 1,989
    // 2,972
    // 3,964
    // 4,972
    std::string header;
    std::getline(in, header);

    // first line with values:
    std::string line;
    std::getline(in, line);
    const size_t firstComma = line.find(',');
    const size_t secondComma = line.find(',', firstComma + 1);
    const size_t thirdComma = line.find(',', secondComma + 1);
    const size_t fourthComma = line.find(',', thirdComma + 1);
    const size_t fifthComma = line.find(',', fourthComma + 1);

    const size_t size = std::stoull(line.substr(fourthComma + 1, fifthComma - (fourthComma + 1)));
    // the first value needs to be extracted with special care as there exists additional data on
    // this line
    if (size > 0)
    {
        const double firstValue =
            std::stod(line.substr(firstComma + 1, secondComma - (firstComma + 1)));
        sampleData.insert(firstValue);
    }

    for (size_t i{1}; i < size; ++i)
    {
        size_t index;
        char comma;
        double value;
        in >> index >> comma >> value;
        sampleData.insert(value);
    }
}

void saveRawFromSampleGroup(const std::filesystem::path &directory,
                            const SampleGroup<double> &sampleGroup)
{
    std::filesystem::create_directories(directory);
    constexpr unsigned NO_THREADS = 8;
    std::vector<std::thread> threads;
    threads.reserve(NO_THREADS);
    const unsigned FILES_PER_THREAD = sampleGroup.size() / NO_THREADS;
    const unsigned REMAINING_FILES = sampleGroup.size() % NO_THREADS;

    auto threadBlock = [&](unsigned threadNo, unsigned filesNo)
    {
        for (unsigned file{0}; file < filesNo; ++file)
        {
            const unsigned byteValue = file + threadNo * filesNo;
            std::filesystem::path filename =
                directory / ("Value_" + std::to_string(byteValue) + ".csv");
            saveRawFromSampleData(filename, sampleGroup[byteValue]);

            DistributionByteValue distribution{sampleGroup[byteValue]};
            filename =
                directory / ("Value_" + std::to_string(byteValue) + "_Distribution" + ".csv");
            saveDistributionByteValue(filename, distribution);
        }
    };

    for (unsigned thread{0}; thread < NO_THREADS - 1; thread++)
        threads.emplace_back(threadBlock, thread, FILES_PER_THREAD);
    // The last thread will also be responsible for the remaining number files.
    threads.emplace_back(threadBlock, NO_THREADS - 1, FILES_PER_THREAD + REMAINING_FILES);

    for (unsigned thread{0}; thread < NO_THREADS; ++thread)
        threads[thread].join();
}

void loadRawFromSampleGroup(const std::filesystem::path &directory,
                            SampleGroup<double> &sampleGroup)
{
    constexpr unsigned NO_THREADS = 8;
    std::vector<std::thread> threads;
    threads.reserve(NO_THREADS);
    const unsigned FILES_PER_THREAD = sampleGroup.size() / NO_THREADS;
    const unsigned REMAINING_FILES = sampleGroup.size() % NO_THREADS;

    auto threadBlock = [&](unsigned threadNo, unsigned filesNo)
    {
        for (unsigned file{0}; file < filesNo; ++file)
        {
            const unsigned byteValue = file + threadNo * filesNo;
            const std::filesystem::path filename =
                directory / ("Value_" + std::to_string(byteValue) + ".csv");
            SampleData<double> result{};
            loadRawFromSampleData(filename, result);
            sampleGroup.insert(byteValue, result.begin(), result.end());
        }
    };

    for (unsigned thread{0}; thread < NO_THREADS - 1; thread++)
        threads.emplace_back(threadBlock, thread, FILES_PER_THREAD);
    // The last thread will also be responsible for the remaining number of files.
    threads.emplace_back(threadBlock, NO_THREADS - 1, FILES_PER_THREAD + REMAINING_FILES);

    for (unsigned thread{0}; thread < NO_THREADS; ++thread)
        threads[thread].join();
}

template <bool KnownKey>
void saveRawFromTimingData(const std::filesystem::path &directory,
                           const TimingData<KnownKey> &timingData)
{
    std::filesystem::create_directories(directory);
    constexpr unsigned NO_THREADS = 8;
    std::vector<std::thread> threads;
    threads.reserve(NO_THREADS);
    const unsigned DIRS_PER_THREAD = timingData.blockTimings.size() / NO_THREADS;
    const unsigned REMAINING_DIRS = timingData.blockTimings.size() % NO_THREADS;

    auto threadBlock = [&](unsigned threadNo, unsigned dirsNo)
    {
        for (unsigned dir{0}; dir < dirsNo; ++dir)
        {
            const unsigned byteValue = dir + threadNo * dirsNo;
            const std::filesystem::path directoryPath =
                directory / ("Byte_" + std::to_string(byteValue));
            saveRawFromSampleGroup(directoryPath, timingData.blockTimings[byteValue]);
        }
    };

    for (unsigned thread{0}; thread < NO_THREADS - 1; thread++)
        threads.emplace_back(threadBlock, thread, DIRS_PER_THREAD);
    // The last thread will also be responsible for the remaining number of directories.
    threads.emplace_back(threadBlock, NO_THREADS - 1, DIRS_PER_THREAD + REMAINING_DIRS);

    // Storing additional information about the timing data.
    // This file will be ignored when loadRawFromTimingData is called().
    std::ofstream out;
    out.open(directory / "info.txt");
    out << "Data Size: " << timingData.dataSize() << '\n';
    if constexpr (KnownKey)
    {
        out << "Key: ";
        for (std::byte byte : timingData.key())
            out << std::hex << std::uppercase << static_cast<unsigned>(byte) << ' ';
        out << '\n';
    }
    for (unsigned thread{0}; thread < NO_THREADS; ++thread)
        threads[thread].join();
}

template <bool KnownKey>
void loadRawFromTimingData(const std::filesystem::path &directory, TimingData<KnownKey> &timingData)
{
    constexpr unsigned NO_THREADS = 8;
    std::vector<std::thread> threads;
    threads.reserve(NO_THREADS);
    const unsigned DIRS_PER_THREAD = timingData.blockTimings.size() / NO_THREADS;
    const unsigned REMAINING_DIRS = timingData.blockTimings.size() % NO_THREADS;

    auto threadBlock = [&](unsigned threadNo, unsigned dirsNo)
    {
        for (unsigned dir{0}; dir < dirsNo; ++dir)
        {
            const unsigned byteValue = dir + threadNo * dirsNo;
            const std::filesystem::path directoryPath =
                directory / ("Byte_" + std::to_string(byteValue));
            loadRawFromSampleGroup(directoryPath, timingData.blockTimings[byteValue]);
        }
    };

    for (unsigned thread{0}; thread < NO_THREADS - 1; thread++)
        threads.emplace_back(threadBlock, thread, DIRS_PER_THREAD);
    // The last thread will also be responsible for the remaining number of directories.
    threads.emplace_back(threadBlock, NO_THREADS - 1, DIRS_PER_THREAD + REMAINING_DIRS);

    for (unsigned thread{0}; thread < NO_THREADS; ++thread)
        threads[thread].join();
}

// EXPLICIT TEMPLATE INSTANTIATION:
template void saveRawFromSampleData<double>(const std::filesystem::path &samplePath,
                                            const SampleData<double> &sampleData);

template void saveRawFromSampleData<size_t>(const std::filesystem::path &samplePath,
                                            const SampleData<size_t> &sampleData);

template void saveMetricsFromTimingData<true>(const std::filesystem::path &filename,
                                              const TimingData<true> &timingData);
template void saveMetricsFromTimingData<false>(const std::filesystem::path &filename,
                                               const TimingData<false> &timingData);

template void saveRawFromTimingData<true>(const std::filesystem::path &filename,
                                          const TimingData<true> &timingData);
template void saveRawFromTimingData<false>(const std::filesystem::path &filename,
                                           const TimingData<false> &timingData);

template void loadRawFromTimingData<true>(const std::filesystem::path &filename,
                                          TimingData<true> &timingData);
template void loadRawFromTimingData<false>(const std::filesystem::path &filename,
                                           TimingData<false> &timingData);

} // namespace SaveLoad
