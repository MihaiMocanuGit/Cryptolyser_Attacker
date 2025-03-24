#include "Study.hpp"

#include <random>

namespace
{
std::vector<std::byte> constructRandomVector(size_t dataSize)
{
    static std::mt19937 gen(0);
    static std::uniform_int_distribution<uint8_t> uniform_dist(0, 255);
    std::vector<std::byte> randomized;
    randomized.reserve(dataSize);
    for (size_t i{0}; i < dataSize; ++i)
        randomized.push_back(static_cast<std::byte>(uniform_dist(gen)));
    return randomized;
}
std::array<std::byte, PACKET_KEY_BYTE_SIZE> constructRandomKey()
{
    static std::mt19937 gen(0);
    static std::uniform_int_distribution<uint8_t> uniform_dist(0, 255);
    std::array<std::byte, PACKET_KEY_BYTE_SIZE> randomized{};
    for (size_t i{0}; i < PACKET_KEY_BYTE_SIZE; ++i)
        randomized[i] = static_cast<std::byte>(uniform_dist(gen));
    return randomized;
}
} // namespace

template <bool KnownKey>
std::pair<double, double> Study<KnownKey>::calibrateBounds(size_t transmissionsCount,
                                                           double confidenceLB, double confidenceUB)
{
    if (m_gatherer.connection().connect())
    {
        SampleData<double> sample;
        sample.reserve(transmissionsCount);
        for (size_t currentCount{0}; currentCount < transmissionsCount && m_continueRunningFlag;
             ++currentCount)
        {
            std::vector<std::byte> studyPlaintext{
                constructRandomVector(m_gatherer.timingData().dataSize())};

            std::optional<connection_response_t> result;
            if constexpr (KnownKey)
            {
                std::array<std::byte, PACKET_KEY_BYTE_SIZE> studiedKey{constructRandomKey()};
                result = m_gatherer.connection().transmit(-1, studiedKey, studyPlaintext);
            }
            else
                result = m_gatherer.connection().transmit(-1, studyPlaintext);

            if (not result)
            {
                std::cerr << "Lost packet." << std::endl;
                // restart the connection
                m_gatherer.connection().closeConnection();
                m_gatherer.connection().connect();
                currentCount--;
                continue;
            }
            const double timing{TimingProcessing::computeDT<double>(
                result->inbound_t1, result->inbound_t2, result->outbound_t1, result->outbound_t2)};
            sample.insert(timing);
        }

        const double max = sample.metrics().max;
        const double min = sample.metrics().min;
        const size_t blockCount{static_cast<size_t>(max + 1.0)};

        // using double instead of size_t because saveRawFromSampleData() was only defined for
        // SampleData<double>
        std::vector<double> blockGroup(blockCount, 0);
        for (const double value : sample)
            blockGroup[static_cast<size_t>(value)]++;

        double lb{min};
        double ub{max};
        double partialSum{0};
        const size_t totalSum{transmissionsCount};
        for (size_t pos{static_cast<size_t>(min) - 1}; pos < static_cast<size_t>(max) + 1; pos++)
        {
            partialSum += blockGroup[pos];
            double ratio = partialSum / static_cast<double>(totalSum);
            if (ratio <= confidenceLB)
            {
                lb = static_cast<double>(pos);
            }
            if (1.0 - ratio <= confidenceUB)
            {
                ub = static_cast<double>(pos);
                break;
            }
        }

        SaveLoad::saveRawFromSampleData(m_saveDirPath / "Calibrate" / "values.csv", sample);

        SampleData<double> savedDistrib(std::move(blockGroup));
        SaveLoad::saveRawFromSampleData(m_saveDirPath / "Calibrate" / "distribution.csv",
                                        savedDistrib);

        return {lb, ub};
    }

    return {0, std::numeric_limits<double>::max()};
}

template <bool KnownKey>
Study<KnownKey>::Study(Gatherer<KnownKey> &&gatherer,
                       const volatile sig_atomic_t &continueRunningFlag,
                       const std::filesystem::path &saveDirPath)
    : m_gatherer{std::move(gatherer)}, m_logger{m_gatherer},
      m_continueRunningFlag{continueRunningFlag}, m_saveDirPath{saveDirPath}
{
}

template <bool KnownKey>
void Study<KnownKey>::run(size_t desiredCount, size_t logFreq, size_t saveMetricsFreq, double lb,
                          double ub)
{
    m_gatherer.init(lb, ub);
    m_logger.init(desiredCount);
    std::filesystem::create_directories(m_saveDirPath);
    uint32_t packageId{0};
    while (m_gatherer.validValuesCount() < desiredCount && m_continueRunningFlag)
    {
        auto status{m_gatherer.obtain(packageId)};

        auto isTimeTo = [this, &gatherer = m_gatherer, desiredCount](size_t freq)
        {
            size_t packageCount{gatherer.validValuesCount()};
            return (packageCount % freq == 0 and packageCount > 0) or
                   packageCount + 1 == desiredCount or not m_continueRunningFlag;
        };

        if (status == Gatherer<KnownKey>::ObtainStatus::success)
        {
            if (isTimeTo(logFreq))
                m_logger.printStats();

            if (isTimeTo(saveMetricsFreq))
            {
                std::cout << "Saving Metrics: " << m_gatherer.validValuesCount() << std::endl;
                SaveLoad::saveMetricsFromTimingData(
                    m_saveDirPath / std::to_string(m_gatherer.validValuesCount()),
                    m_gatherer.timingData());
            }
        }
        packageId++;
    }

    SaveLoad::saveRawFromSampleData(m_saveDirPath / "Raw" / "LB.csv", m_gatherer.sampleLB());
    SaveLoad::saveRawFromSampleData(m_saveDirPath / "Raw" / "UB.csv", m_gatherer.sampleUB());
    SaveLoad::saveRawFromTimingData(m_saveDirPath / "Raw", m_gatherer.timingData());
}

template <bool KnownKey>
Gatherer<KnownKey> Study<KnownKey>::release()
{
    Gatherer<KnownKey> forceRelease = std::move(m_gatherer);
    return forceRelease;
}

template class Study<true>;
template class Study<false>;
