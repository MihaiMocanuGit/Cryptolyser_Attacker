#include "Logger.hpp"

#include <iomanip>

template <bool KnownKey>
void Logger<KnownKey>::printStats()
{
    const double completionPercent{static_cast<double>(m_totalValidCount()) /
                                   static_cast<double>(m_finalValidCount) * 100.0};
    const double invalidPercent{100.0 - static_cast<double>(m_totalValidCount()) /
                                            static_cast<double>(m_totalSentCount()) * 100.0};

    timespec currentTime{};
    clock_gettime(CLOCK_MONOTONIC, &currentTime);
    double totalElapsedTime{
        TimingProcessing::computeDT<double>(m_startStudyTime.tv_sec, m_startStudyTime.tv_nsec,
                                            currentTime.tv_sec, currentTime.tv_nsec)};
    totalElapsedTime *= 1.0e-9; // nanosec to sec
    // Estimated time until completion in minutes
    const double ETA = (100.0 - completionPercent) * static_cast<double>(totalElapsedTime) /
                       (completionPercent * 60.0);

    double passTime{TimingProcessing::computeDT<double>(
        m_prevPassTime.tv_sec, m_prevPassTime.tv_nsec, currentTime.tv_sec, currentTime.tv_nsec)};
    passTime *= 1.0e-9; // nanosec to sec
    const double rate{static_cast<double>(m_gatherer.validValuesCount() - m_prevPassPacketCount) /
                      passTime};

    m_prevPassTime = currentTime;
    m_prevPassPacketCount = m_gatherer.validValuesCount();
    // TODO: convert to fixed width columns
    std::cout << "Stats:\n"                                                         //
              << "\tETA: " << ETA << " minutes"                                     //
              << "\t Progress: " << m_totalValidCount() << '/' << m_finalValidCount //
              << " (" << completionPercent << "%)"                                  //
              << "\t Study Rate: " << rate << " packets/second"                     //
              << "\t Invalid count: " << m_totalSentCount() - m_totalValidCount()   //
              << " (" << invalidPercent << "%)"                                     //
              << '\n';                                                              //

    const auto &blockTimings = m_gatherer.timingData().blockTimings;
    const size_t avgSampleSize{m_totalValidCount() / 256};
    double min = blockTimings[0].globalMetrics().min;
    double max = blockTimings[0].globalMetrics().max;
    for (const auto &sampleGroup : blockTimings)
    {
        min = std::min(min, sampleGroup.globalMetrics().min);
        max = std::max(max, sampleGroup.globalMetrics().max);
    }

    std::cout << "Sample Group:" << '\n'; //
    if (m_totalValidCount() == 0)
        std::cout << "\tEmpty.\n";
    else
    {
        std::cout << std::fixed << std::setprecision(3)                     //
                  << "\tMean: " << blockTimings[0].globalMetrics().mean     //
                  << "\tStdDev: " << blockTimings[0].globalMetrics().stdDev //
                  << "\tAverage Sample Size: " << avgSampleSize             //
                  << "\tMin: " << min                                       //
                  << "\tMax: " << max                                       //
                  << '\n';                                                  //
    }

    auto printIgnoredBound = [this](double bound, const SampleData<double> &sample)
    {
        float ratio = static_cast<float>(sample.metrics().size) / m_totalReceivedCount() * 100.0;
        std::cout << "Values for bound: " << bound << '\n';
        if (sample.metrics().size == 0)
            std::cout << "\tEmpty.\n";
        else
        {
            std::cout << std::fixed << std::setprecision(3)                           //
                      << "\tMean: " << sample.metrics().mean                          //
                      << "\tStdDev: " << sample.metrics().stdDev                      //
                      << "\tSize: " << sample.metrics().size << " (" << ratio << "%)" //
                      << "\tMin: " << sample.metrics().min                            //
                      << "\tMax: " << sample.metrics().max                            //
                      << '\n';                                                        //
        }
    };

    printIgnoredBound(m_gatherer.lb(), m_gatherer.sampleLB());
    printIgnoredBound(m_gatherer.ub(), m_gatherer.sampleUB());

    std::cout << '\n';
}

template <bool KnownKey>
void Logger<KnownKey>::init(size_t finalValidValuesCount)
{
    m_finalValidCount = finalValidValuesCount;

    clock_gettime(CLOCK_MONOTONIC, &m_startStudyTime);
    m_prevPassTime = m_startStudyTime;
}

template <bool KnownKey>
Logger<KnownKey>::Logger(const Gatherer<KnownKey> &gatherer) noexcept : m_gatherer{gatherer}
{
}

template <bool KnownKey>
size_t Logger<KnownKey>::m_totalValidCount() const
{
    return m_gatherer.validValuesCount();
}

template <bool KnownKey>
size_t Logger<KnownKey>::m_totalReceivedCount() const
{
    return m_totalValidCount() + m_gatherer.sampleLB().size() + m_gatherer.sampleUB().size();
}

template <bool KnownKey>
size_t Logger<KnownKey>::m_totalSentCount() const
{
    return m_totalReceivedCount() + m_gatherer.lostPackages();
}

template class Logger<true>;
template class Logger<false>;
