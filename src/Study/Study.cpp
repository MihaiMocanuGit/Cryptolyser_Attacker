#include "Study.hpp"

#include "DataProcessing/Timings/TimingProcessing.hpp"

#include <filesystem>
#include <format>
#include <iostream>
#include <thread>

namespace
{
std::vector<std::byte> m_constructRandomVector(size_t size)
{
    static std::mt19937 gen(0);
    static std::uniform_int_distribution<uint8_t> uniform_dist(0, 255);
    std::vector<std::byte> randomized;
    randomized.reserve(size);
    for (size_t i{0}; i < size; ++i)
        randomized.push_back(static_cast<std::byte>(uniform_dist(gen)));
    return randomized;
}
} // namespace

struct Study::StudyContext
{
    struct State
    {
        size_t currentCount{0};
        size_t currentId{0};
        size_t lostNetworkPackages{0};

        timespec startStudyTime{};
        timespec prevPassTime{.tv_sec = 0, .tv_nsec = 0};
        size_t prevPassPacketCount{0};
    } state;

    const size_t AVG_SAMPLE_SIZE;
    const size_t APPROX_TOTAL_COUNT{Study::AES_BLOCK_SIZE * Study::SAMPLE_GROUP_SIZE *
                                    AVG_SAMPLE_SIZE};
    struct Display
    {
        const size_t PRINT_FREQ;
        const size_t SAVE_FREQ;
        const std::string SAVE_FOLDER_PATH;

        Display(size_t printFreq, size_t saveFreq, std::string savePath)
            : PRINT_FREQ{printFreq}, SAVE_FREQ{saveFreq}, SAVE_FOLDER_PATH{savePath}
        {
        }

        Display(const Study::DisplayParams &display)
            : Display{display.printFreq, display.saveFreq, display.savePath}
        {
        }
    } display;
    struct Boundary
    {
        const double TIMING_LB;
        const double TIMING_UB;
        SampleData<double> sampleLB{};
        SampleData<double> sampleUB{};

        Boundary(double timingLB, double timingUB) : TIMING_LB{timingLB}, TIMING_UB{timingUB} {}
        Boundary(const Study::TimingBoundaryParams &bounds) : Boundary{bounds.lb, bounds.ub} {}
    } bd;

    StudyContext(size_t avgSampleSize, const Study::DisplayParams &display,
                 const Study::TimingBoundaryParams &bounds)
        : AVG_SAMPLE_SIZE{avgSampleSize}, display{display}, bd{bounds}
    {
    }
};

size_t Study::m_totalCount(const StudyContext &ctx) const
{
    const StudyContext::State &state = ctx.state;
    const StudyContext::Boundary &bd = ctx.bd;
    return ctx.APPROX_TOTAL_COUNT + state.lostNetworkPackages + bd.sampleLB.data().size() +
           bd.sampleUB.data().size();
}

bool Study::filterCurrentValue(StudyContext &ctx, double timing)
{
    StudyContext::Boundary &bd = ctx.bd;
    if (timing < bd.TIMING_LB)
    {
        bd.sampleLB.insert(timing);
        return true;
    }
    if (timing > bd.TIMING_UB)
    {
        bd.sampleUB.insert(timing);
        return true;
    }
    return false;
}

void Study::printStats(StudyContext &ctx)
{
    StudyContext::State &state = ctx.state;
    StudyContext::Display &display = ctx.display;
    StudyContext::Boundary &bd = ctx.bd;
    if ((state.currentCount != 0 and state.currentCount % display.PRINT_FREQ == 0) or
        state.currentCount + 1 == m_totalCount(ctx))
    {

        const double completionPercent{static_cast<double>(state.currentCount) /
                                       static_cast<double>(m_totalCount(ctx)) * 100.0};

        timespec currentTime{};
        clock_gettime(CLOCK_MONOTONIC, &currentTime);
        double totalElapsedTime{TimingProcessing::computeDT<double>(
            state.startStudyTime.tv_sec, state.startStudyTime.tv_nsec, currentTime.tv_sec,
            currentTime.tv_nsec)};
        totalElapsedTime *= 1.0e-9; // nanosec to sec
        // Estimated time until completion in minutes
        const double ETA = (100.0 - completionPercent) * static_cast<double>(totalElapsedTime) /
                           (completionPercent * 60.0);

        double passTime{TimingProcessing::computeDT<double>(
            state.prevPassTime.tv_sec, state.prevPassTime.tv_nsec, currentTime.tv_sec,
            currentTime.tv_nsec)};
        passTime *= 1.0e-9; // nanosec to sec
        const double rate{static_cast<double>(state.currentCount - state.prevPassPacketCount) /
                          passTime};

        state.prevPassTime = currentTime;
        state.prevPassPacketCount = state.currentCount;
        // TODO: convert to fixed width columns
        std::cout << "Stats:\n"                                                            //
                  << "\tETA: " << ETA << " minutes"                                        //
                  << "\t Progress: " << state.currentCount + 1 << '/' << m_totalCount(ctx) //
                  << " (" << completionPercent << "%)"                                     //
                  << "\t Study Rate: " << rate << " packets/second"                        //
                  << '\n';                                                                 //

        const size_t avgSampleSize{m_sampleGroups[0].globalMetrics().size / SAMPLE_GROUP_SIZE};
        double min = m_sampleGroups[0].globalMetrics().min;
        double max = m_sampleGroups[0].globalMetrics().max;
        for (const auto &sampleGroup : m_sampleGroups)
        {
            min = std::min(min, sampleGroup.globalMetrics().min);
            max = std::max(max, sampleGroup.globalMetrics().max);
        }
        std::cout << "Sample Group:" << '\n'; //
        if (m_sampleGroups[0].size() == 0)
            std::cout << "\tEmpty.\n";
        else
        {
            std::cout << std::fixed << std::setprecision(3)                       //
                      << "\tMean: " << m_sampleGroups[0].globalMetrics().mean     //
                      << "\tStdDev: " << m_sampleGroups[0].globalMetrics().stdDev //
                      << "\tAverage Sample Size: " << avgSampleSize               //
                      << "\tMin: " << min                                         //
                      << "\tMax: " << max                                         //
                      << '\n';                                                    //
        }

        float ratio = static_cast<float>(bd.sampleLB.metrics().size) / state.currentCount * 100.0;
        std::cout << "LB values for: " << bd.TIMING_LB << '\n';
        if (bd.sampleLB.metrics().size == 0)
            std::cout << "\tEmpty.\n";
        else
        {
            std::cout << std::fixed << std::setprecision(3)                                //
                      << "\tMean: " << bd.sampleLB.metrics().mean                          //
                      << "\tStdDev: " << bd.sampleLB.metrics().stdDev                      //
                      << "\tSize: " << bd.sampleLB.metrics().size << " (" << ratio << "%)" //
                      << "\tMin: " << bd.sampleLB.metrics().min                            //
                      << "\tMax: " << bd.sampleLB.metrics().max                            //
                      << '\n';                                                             //
        }

        ratio = static_cast<float>(bd.sampleUB.metrics().size) / state.currentCount * 100.0;
        std::cout << "UB values for: " << bd.TIMING_UB << '\n';
        if (bd.sampleUB.metrics().size == 0)
            std::cout << "\tEmpty.\n";
        else
        {
            std::cout << std::fixed << std::setprecision(3)                                //
                      << "\tMean: " << bd.sampleUB.metrics().mean                          //
                      << "\tStdDev: " << bd.sampleUB.metrics().stdDev                      //
                      << "\tSize: " << bd.sampleUB.metrics().size << " (" << ratio << "%)" //
                      << "\tMin: " << bd.sampleUB.metrics().min                            //
                      << "\tMax: " << bd.sampleUB.metrics().max                            //
                      << '\n';                                                             //
        }

        std::cout << '\n';
    }
}

namespace
{

void saveRawByteBlockThread(std::string directory, unsigned byteBlock,
                            const SampleGroup<double> &sampleGroup)
{
    std::string currentLevelPath{directory + "/Raw/Byte_" + std::to_string(byteBlock)};
    std::filesystem::create_directory(currentLevelPath);
    for (unsigned value{0}; value < sampleGroup.size(); ++value)
    {
        std::ofstream out;
        out.open(currentLevelPath + "/Value_" + std::to_string(value) + ".csv");
        if (!out)
            throw std::runtime_error("Saving Raw Data | Could not create file: " +
                                     currentLevelPath + "/Value_" + std::to_string(value) + ".csv");

        const auto &sampleData{sampleGroup[value].data()};
        out << "INDICES, VALUES, SIZE\n";
        if (!sampleData.empty())
        {
            out << 0 << ", " << sampleData[0] << ", " << sampleData.size() << '\n';
        }
        for (size_t i{1}; i < sampleData.size(); ++i)
        {
            out << i << ", " << sampleData[i] << ",\n";
        }
    }
}

void saveMetricsByteBlockThread(std::string directory, unsigned byteBlock,
                                const SampleGroup<double> &sampleGroup)
{
    const std::string filepath{directory + "/" + std::to_string(byteBlock) + ".csv"};
    std::ofstream out;
    out.open(filepath);
    if (!out)
        throw std::runtime_error("Saving Metrics Data | Could not create file: " + filepath);

    constexpr std::string_view header{
        "Value, Mean, StdDev, Size, StandardizedMean, StandardizedStdDev, Min, Max\n"};
    out << header;
    for (unsigned byteValue = 0; byteValue < sampleGroup.size(); ++byteValue)
    {
        SampleMetrics metrics = sampleGroup.localMetrics(byteValue);
        SampleMetrics standardizedMetrics = sampleGroup.standardizeLocalMetrics(byteValue);
        out << static_cast<int>(static_cast<uint8_t>(byteValue)) << ", " //
            << std::setprecision(8) << std::fixed                        //
            << metrics.mean << ", "                                      //
            << metrics.stdDev << ", "                                    //
            << metrics.size << ", "                                      //
            << standardizedMetrics.mean << ", "                          //
            << standardizedMetrics.stdDev << ", "                        //
            << metrics.min << ", "                                       //
            << metrics.max << "\n";                                      //
    }
    out.close();
}

void loadByteBlockThread(std::string byteBlockDir, SampleGroup<double> &sampleGroup)
{
    for (unsigned byteValue{0}; byteValue < Study::SAMPLE_GROUP_SIZE; ++byteValue)
    {
        std::ifstream in;
        in.open(byteBlockDir + "/Value_" + std::to_string(byteValue) + ".csv");
        if (!in)
            throw std::runtime_error("Loading from Data | Could not create open file: " +
                                     byteBlockDir + "/Value_" + std::to_string(byteValue) + ".csv");
        // Example file:
        // INDICES, VALUES, SIZE
        // 0, 972, 65085
        // 1, 989,
        // 2, 972,
        // 3, 964,
        // 4, 972,
        std::vector<double> sampleData;
        std::string header;
        std::getline(in, header);

        // first line with values:
        size_t index, value, size;
        char comma1, comma2;
        in >> index >> comma1 >> value >> comma2 >> size;
        sampleData.reserve(size);
        sampleData.push_back(value);
        for (size_t i{1}; i < size; ++i)
        {
            in >> index >> comma1 >> value >> comma2;
            sampleData.push_back(value);
        }
        sampleGroup.insert(byteValue, sampleData.begin(), sampleData.end());
    }
}

} // namespace

void Study::saveDataRaw(const std::string &directory, const std::vector<SampleGroup<double>> &data)
{

    std::filesystem::create_directory(directory);
    std::filesystem::create_directory(directory + "/Raw");
    std::vector<std::thread> threads;
    threads.reserve(AES_BLOCK_SIZE);
    for (unsigned byteBlock{0}; byteBlock < AES_BLOCK_SIZE; ++byteBlock)
    {
        // Doesn't use mutexes as every sampleGroup is independent of the other.
        threads.emplace_back(saveRawByteBlockThread, directory, byteBlock,
                             std::ref(data[byteBlock]));
    }

    for (unsigned byteBlock{0}; byteBlock < AES_BLOCK_SIZE; ++byteBlock)
    {
        threads[byteBlock].join();
    }
}

void Study::saveDataMetrics(const std::string &directory,
                            const std::vector<SampleGroup<double>> &data)
{
    std::filesystem::create_directory(directory);
    std::vector<std::thread> threads;
    threads.reserve(AES_BLOCK_SIZE);
    for (unsigned byteBlock{0}; byteBlock < AES_BLOCK_SIZE; ++byteBlock)
    {
        threads.emplace_back(saveMetricsByteBlockThread, directory, byteBlock,
                             std::ref(data[byteBlock]));
    }

    for (unsigned byteBlock{0}; byteBlock < AES_BLOCK_SIZE; ++byteBlock)
    {
        threads[byteBlock].join();
    }
}

void Study::loadPreviousStudyData(const std::string &prevRawDir)
{
    std::vector<std::thread> threads;
    threads.reserve(AES_BLOCK_SIZE);
    for (unsigned byteBlock{0}; byteBlock < AES_BLOCK_SIZE; ++byteBlock)
    {
        // Doesn't use mutexes as every sampleGroup is independent of the other.
        std::string currentLevelPath{prevRawDir + "/Byte_" + std::to_string(byteBlock)};
        threads.emplace_back(loadByteBlockThread, currentLevelPath,
                             std::ref(m_sampleGroups[byteBlock]));
    }

    for (unsigned byteBlock{0}; byteBlock < AES_BLOCK_SIZE; ++byteBlock)
    {
        threads[byteBlock].join();
    }
}

bool Study::isSaveTime(const StudyContext &ctx) const
{
    return (ctx.state.currentCount != 0 and ctx.state.currentCount % ctx.display.SAVE_FREQ == 0) or
           ctx.state.currentCount + 1 == m_totalCount(ctx) or not m_continueRunning;
}

Study::Study(ServerConnection &&connection, volatile const int &continueRunningFlag,
             size_t dataPacketLength)
    : m_DATA_PACKET_LENGTH{dataPacketLength}, m_connection{std::move(connection)},
      m_continueRunning{continueRunningFlag}
{
}

void Study::start(size_t desiredAvgSampleSize, const DisplayParams &displayParams,
                  const TimingBoundaryParams &bounds)
{
    if (m_connection.connect())
    {
        StudyContext ctx(desiredAvgSampleSize, displayParams, bounds);
        StudyContext::State &state = ctx.state;
        StudyContext::Display &display = ctx.display;

        for (auto &sampleGroup : m_sampleGroups)
        {
            constexpr float assumedDeviationFactor = 1.15;
            sampleGroup.reserveForAll(ctx.AVG_SAMPLE_SIZE * assumedDeviationFactor);
        }

        clock_gettime(CLOCK_MONOTONIC, &state.startStudyTime);
        state.prevPassTime = state.startStudyTime;
        for (state.currentCount = 0; state.currentCount < m_totalCount(ctx) && m_continueRunning;
             ++state.currentCount)
        {
            this->printStats(ctx);
            if (isSaveTime(ctx))
            {
                std::cout << "Writing Metrics Data: " << state.currentCount << "\n\n";
                const std::string saveDir{display.SAVE_FOLDER_PATH + "/" +
                                          std::to_string(state.currentCount)};
                Study::saveDataMetrics(saveDir, m_sampleGroups);
            }

            std::vector<std::byte> studyPlaintext = m_constructRandomVector(m_DATA_PACKET_LENGTH);
            const auto result{m_connection.transmit(state.currentId, studyPlaintext)};
            if (not result)
            {
                std::cerr << "Lost packet with id:\t" << state.currentId << " Loss rate: "
                          << static_cast<double>(++state.lostNetworkPackages) /
                                 (static_cast<double>(state.currentCount + 1))
                          << std::endl;
                // restart the connection
                m_connection.closeConnection();
                m_connection.connect();
                continue;
            }
            const double timing{TimingProcessing::computeDT<double>(
                result->inbound_t1, result->inbound_t2, result->outbound_t1, result->outbound_t2)};
            if (not filterCurrentValue(ctx, timing))
            {
                state.currentId++;
                for (unsigned byteIndex{0};
                     byteIndex < AES_BLOCK_SIZE and byteIndex < m_DATA_PACKET_LENGTH; ++byteIndex)
                    m_sampleGroups[byteIndex].insert(static_cast<size_t>(studyPlaintext[byteIndex]),
                                                     timing);
            }
        }

        std::cout << "Writing Final Metrics Data: " << state.currentCount << "\n\n";
        const std::string saveDir{display.SAVE_FOLDER_PATH + "/" + "Final"};
        Study::saveDataMetrics(saveDir, m_sampleGroups);

        std::cout << "Writing Raw Data: " << state.currentCount << "\n\n";
        Study::saveDataRaw(display.SAVE_FOLDER_PATH, m_sampleGroups);
    }
}
const std::vector<SampleGroup<double>> &Study::data() const { return m_sampleGroups; }

Study::TimingBoundaryParams Study::calibrate(const std::string &saveTo, size_t transmissionsCount,
                                             double confidenceLB, double confidenceUB)
{
    if (m_connection.connect())
    {
        SampleData<double> sample;
        sample.reserve(transmissionsCount);
        std::vector<std::byte> studyPlaintext{m_DATA_PACKET_LENGTH};
        for (size_t currentCount{0}; currentCount < transmissionsCount && m_continueRunning;
             ++currentCount)
        {
            const auto result{m_connection.transmit(-1, studyPlaintext)};
            if (not result)
            {
                std::cerr << "Lost packet." << std::endl;
                // restart the connection
                m_connection.closeConnection();
                m_connection.connect();
                currentCount--;
                continue;
            }
            const double timing{TimingProcessing::computeDT<double>(
                result->inbound_t1, result->inbound_t2, result->outbound_t1, result->outbound_t2)};
            sample.insert(timing);
        }

        const double max = sample.metrics().max;
        const double min = sample.metrics().min;
        const size_t blockCount{static_cast<size_t>(max - min + 1.0)};

        std::vector<unsigned> blockGroup(blockCount, 0);
        for (const double value : sample)
        {
            size_t index{static_cast<size_t>(value - min)};
            assert(index < blockCount);
            blockGroup[index]++;
        }
        double lb{min};
        double ub{max};
        size_t partialSum{0};
        const size_t totalSum{transmissionsCount};
        for (size_t pos{0}; pos < blockCount; pos++)
        {
            partialSum += blockGroup[pos];
            double ratio = static_cast<double>(partialSum) / static_cast<double>(totalSum);
            if (ratio <= confidenceLB)
            {
                lb = min + static_cast<double>(pos);
            }
            if (1.0 - ratio <= confidenceUB)
            {
                ub = min + static_cast<double>(pos);
                break;
            }
        }

        std::ofstream out;
        out.open(saveTo + "/Distribution.csv");
        for (size_t i{0}; i < static_cast<size_t>(min); ++i)
        {
            out << 0 << ",\n";
        }
        for (const auto &block : blockGroup)
        {
            out << block << ",\n";
        }

        return {.lb = lb, .ub = ub};
    }

    return {0, std::numeric_limits<double>::max()};
}
