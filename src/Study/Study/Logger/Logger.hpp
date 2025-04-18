#ifndef CRYPTOLYSER_ATTACKER_LOGGER_HPP
#define CRYPTOLYSER_ATTACKER_LOGGER_HPP

#include "DataProcessing/Timings/TimingProcessing.hpp"
#include "Study/Gatherer/Gatherer.hpp"

template <bool KnownKey>
class Logger
{
  private:
    const Gatherer<KnownKey> &m_gatherer;
    timespec m_startStudyTime {};
    timespec m_prevPassTime {.tv_sec = 0, .tv_nsec = 0};
    size_t m_prevPassPacketCount {0};
    size_t m_finalValidCount;
    [[nodiscard]] size_t m_totalSentCount() const;
    [[nodiscard]] size_t m_totalReceivedCount() const;
    [[nodiscard]] size_t m_totalValidCount() const;

  public:
    explicit Logger(const Gatherer<KnownKey> &gatherer) noexcept;
    void init(size_t finalValidValuesCount);
    void printStats();
};

#endif // CRYPTOLYSER_ATTACKER_LOGGER_HPP
