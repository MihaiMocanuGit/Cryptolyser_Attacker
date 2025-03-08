#include "Study.hpp"

#include "DataProcessing/Timings/TimingProcessing.hpp"

#include <filesystem>
#include <format>
#include <iostream>

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

size_t Study::m_totalCount() const
{
    return m_APPROXIMATE_TOTAL_COUNT + m_lostNetworkPackages + m_sampleLB.data().size() +
           m_sampleUB.data().size();
}

bool Study::filterCurrentValue(double timing)
{
    if (timing < m_TIMING_LB)
    {
        m_sampleLB.insert(timing);
        return true;
    }
    if (timing > m_TIMING_UB)
    {
        m_sampleUB.insert(timing);
        return true;
    }
    return false;
}

void Study::printStats()
{
    if ((m_currentCount != 0 and m_currentCount % m_PRINT_FREQ == 0) or
        m_currentCount + 1 == m_totalCount())
    {

        const double completionPercent{static_cast<double>(m_currentCount) /
                                       static_cast<double>(m_totalCount()) * 100.0};

        timespec currentTime{};
        clock_gettime(CLOCK_MONOTONIC, &currentTime);
        double totalElapsedTime{
            TimingProcessing::computeDT<double>(m_startStudyTime.tv_sec, m_startStudyTime.tv_nsec,
                                                currentTime.tv_sec, currentTime.tv_nsec)};
        totalElapsedTime *= 1.0e-9; // nanosec to sec
        // Estimated time until completion in minutes
        const double ETA = (100.0 - completionPercent) * static_cast<double>(totalElapsedTime) /
                           (completionPercent * 60.0);

        double passTime{
            TimingProcessing::computeDT<double>(m_prevPassTime.tv_sec, m_prevPassTime.tv_nsec,
                                                currentTime.tv_sec, currentTime.tv_nsec)};
        passTime *= 1.0e-9; // nanosec to sec
        const double rate{static_cast<double>(m_currentCount - m_prevPassPacketCount) / passTime};

        m_prevPassTime = currentTime;
        m_prevPassPacketCount = m_currentCount;
        // TODO: convert to fixed width columns
        std::cout << "Stats:\n"                                                     //
                  << "\tETA: " << ETA << " minutes"                                 //
                  << "\t Progress: " << m_currentCount + 1 << '/' << m_totalCount() //
                  << " (" << completionPercent << "%)"                              //
                  << "\t Study Rate: " << rate << " packets/second"                 //
                  << '\n';                                                          //

        const size_t avgSampleSize{m_sampleGroups[0].globalMetrics().size / m_SAMPLE_GROUP_SIZE};
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

        float ratio = static_cast<float>(m_sampleLB.metrics().size) / m_currentCount * 100.0;
        std::cout << "LB values for: " << m_TIMING_LB << '\n';
        if (m_sampleLB.metrics().size == 0)
            std::cout << "\tEmpty.\n";
        else
        {
            std::cout << std::fixed << std::setprecision(3)                               //
                      << "\tMean: " << m_sampleLB.metrics().mean                          //
                      << "\tStdDev: " << m_sampleLB.metrics().stdDev                      //
                      << "\tSize: " << m_sampleLB.metrics().size << " (" << ratio << "%)" //
                      << "\tMin: " << m_sampleLB.metrics().min                            //
                      << "\tMax: " << m_sampleLB.metrics().max                            //
                      << '\n';                                                            //
        }

        ratio = static_cast<float>(m_sampleUB.metrics().size) / m_currentCount * 100.0;
        std::cout << "UB values for: " << m_TIMING_UB << '\n';
        if (m_sampleUB.metrics().size == 0)
            std::cout << "\tEmpty.\n";
        else
        {
            std::cout << std::fixed << std::setprecision(3)                               //
                      << "\tMean: " << m_sampleUB.metrics().mean                          //
                      << "\tStdDev: " << m_sampleUB.metrics().stdDev                      //
                      << "\tSize: " << m_sampleUB.metrics().size << " (" << ratio << "%)" //
                      << "\tMin: " << m_sampleUB.metrics().min                            //
                      << "\tMax: " << m_sampleUB.metrics().max                            //
                      << '\n';                                                            //
        }

        std::cout << '\n';
    }
}

void Study::saveMetrics() const
{
    if ((m_currentCount != 0 and m_currentCount % m_SAVE_FREQ == 0) or
        m_currentCount + 1 == m_totalCount() or not m_continueRunning)
    {
        std::cout << "Writing Metrics Data: " << m_currentCount << "\n\n";
        const std::string root{m_SAVE_FOLDER_PATH + "/" + std::to_string(m_currentCount)};
        std::filesystem::create_directory(root);
        for (unsigned i{0}; i < m_AES_BLOCK_SIZE; ++i)
        {
            const std::string filepath{root + "/" + std::to_string(i) + ".csv"};
            std::ofstream out;
            out.open(filepath);
            if (!out)
                throw std::runtime_error("Saving Metrics Data | Could not create file: " +
                                         filepath);

            constexpr std::string_view header{
                "Value, Mean, StdDev, Size, StandardizedMean, StandardizedStdDev, Min, Max\n"};
            out << header;
            for (unsigned byteValue = 0; byteValue < m_sampleGroups[i].size(); ++byteValue)
            {
                SampleMetrics metrics = m_sampleGroups[i].localMetrics(byteValue);
                SampleMetrics standardizedMetrics =
                    m_sampleGroups[i].standardizeLocalMetrics(byteValue);
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
    }
}

void Study::saveRaw() const
{
    std::cout << "Writing Raw Data: " << m_currentCount << "\n\n";
    std::filesystem::create_directory(m_SAVE_FOLDER_PATH + "/Raw");
    for (unsigned byteBlock{0}; byteBlock < m_AES_BLOCK_SIZE; ++byteBlock)
    {
        std::string currentLevelPath{m_SAVE_FOLDER_PATH + "/Raw/Byte_" + std::to_string(byteBlock)};
        std::filesystem::create_directory(currentLevelPath);
        const auto &sampleGroup{m_sampleGroups[byteBlock]};
        for (unsigned value{0}; value < sampleGroup.size(); ++value)
        {
            std::ofstream out;
            out.open(currentLevelPath + "/Value_" + std::to_string(value) + ".csv");
            if (!out)
                throw std::runtime_error("Saving Raw Data | Could not create file: " +
                                         currentLevelPath + "/Value_" + std::to_string(value));

            const auto &data{sampleGroup[value].data()};
            out << "INDICES, VALUES, SIZE\n";
            if (data.size() > 0)
            {
                out << 0 << ", " << data[0] << ", " << data.size() << '\n';
            }
            for (size_t i{1}; i < data.size(); ++i)
            {
                out << i << ", " << data[i] << ",\n";
            }
        }
    }
}

Study::Study(ServerConnection &&connection, const volatile sig_atomic_t &continueRunningFlag,
             const Data &data, const Display &display, const TimingBoundary &bounds)
    : m_DESIRED_MEAN_SAMPLE_SIZE{data.desiredMeanSampleSize},
      m_DATA_PACKET_LENGTH{data.dataPacketLength}, m_connection{std::move(connection)},
      m_continueRunning{continueRunningFlag}, m_SAVE_FOLDER_PATH{display.savePath},
      m_PRINT_FREQ{display.printFreq}, m_SAVE_FREQ{display.saveFreq}, m_TIMING_LB{bounds.lb},
      m_TIMING_UB{bounds.ub}
{
}

void Study::start()
{
    if (m_connection.connect())
    {
        clock_gettime(CLOCK_MONOTONIC, &m_startStudyTime);
        m_prevPassTime = m_startStudyTime;
        for (m_currentCount = 0; m_currentCount < m_totalCount() && m_continueRunning;
             ++m_currentCount)
        {
            this->printStats();
            this->saveMetrics();

            std::vector<std::byte> studyPlaintext = m_constructRandomVector(m_DATA_PACKET_LENGTH);
            const auto result{m_connection.transmit(m_currentId, studyPlaintext)};
            if (not result)
            {
                std::cerr << "Lost packet with id:\t" << m_currentId << " Loss rate: "
                          << static_cast<double>(++m_lostNetworkPackages) /
                                 (static_cast<double>(m_currentCount + 1))
                          << std::endl;
                // restart the connection
                m_connection.closeConnection();
                m_connection.connect();
                continue;
            }
            const double timing{TimingProcessing::computeDT<double>(
                result->inbound_t1, result->inbound_t2, result->outbound_t1, result->outbound_t2)};
            if (not filterCurrentValue(timing))
            {
                m_currentId++;
                for (unsigned byteIndex{0};
                     byteIndex < m_AES_BLOCK_SIZE and byteIndex < m_DATA_PACKET_LENGTH; ++byteIndex)
                    m_sampleGroups[byteIndex].insert(static_cast<size_t>(studyPlaintext[byteIndex]),
                                                     timing);
            }
        }
        this->saveMetrics();
        this->saveRaw();
    }
}
