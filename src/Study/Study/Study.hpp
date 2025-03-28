#ifndef CRYPTOLYSER_ATTACKER_STUDY_HPP
#define CRYPTOLYSER_ATTACKER_STUDY_HPP

#include "Gatherer/Gatherer.hpp"
#include "Logger/Logger.hpp"
#include "SaveLoad/SaveLoad.hpp"

#include <csignal>

template <bool KnownKey>
class Study
{
  private:
    Gatherer<KnownKey> m_gatherer;
    Logger<KnownKey> m_logger;
    const volatile sig_atomic_t &m_continueRunningFlag;
    const std::filesystem::path &m_saveDirPath;

  public:
    Study(Gatherer<KnownKey> &&gatherer, const volatile sig_atomic_t &continueRunningFlag,
          const std::filesystem::path &saveDirPath);

    void run(size_t desiredCount, size_t logFreq, size_t saveMetricsFreq, double lb = 0,
             double ub = std::numeric_limits<double>::max());

    DistributionByteValue::Bounds calibrateBounds(size_t transmissionsCount = 500'000,
                                                  double confidenceLB = 0.0000125,
                                              double confidenceUB = 0.0005);

    Gatherer<KnownKey> release();
};

#endif // CRYPTOLYSER_ATTACKER_STUDY_HPP
