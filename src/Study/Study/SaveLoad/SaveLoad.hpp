#ifndef CRYPTOLYSER_ATTACKER_SAVELOAD_HPP
#define CRYPTOLYSER_ATTACKER_SAVELOAD_HPP
#include "DataProcessing/Distribution/DistributionByteBlock.hpp"
#include "DataProcessing/Distribution/DistributionByteValue.hpp"
#include "DataProcessing/Samples/SampleGroup.hpp"
#include "Study/OldTimingData/TimingData.hpp"

#include <filesystem>

namespace SaveLoad
{
// DISTRIBUTION
void saveDistributionByteValue(const std::filesystem::path &filename,
                               const DistributionByteValue &distribution);

void saveDistributionByteBlock(const std::filesystem::path &filename,
                               const DistributionByteBlock &distribution);

// METRICS SAMPLE GROUP
void saveMetricsFromSampleGroup(const std::filesystem::path &filename,
                                const Old::SampleGroup<double> &sampleGroup);

// METRICS TIMING DATA
template <bool KnownKey>
void saveMetricsFromTimingData(const std::filesystem::path &directory,
                               const Old::TimingData<KnownKey> &timingData);

// RAW SAMPLE DATA
template <typename Real_t>
void saveRawFromSampleData(const std::filesystem::path &samplePath,
                           const Old::SampleData<Real_t> &sampleData);

void loadRawFromSampleData(const std::filesystem::path &filename,
                           Old::SampleGroup<double> &sampleData);

// RAW SAMPLE GROUP
void saveRawFromSampleGroup(const std::filesystem::path &directory,
                            const Old::SampleGroup<double> &sampleGroup);

void loadRawFromSampleGroup(const std::filesystem::path &directory,
                            Old::SampleGroup<double> &sampleGroup);

// RAW TIMING DATA
template <bool KnownKey>
void saveRawFromTimingData(const std::filesystem::path &directory,
                           const Old::TimingData<KnownKey> &timingData);
template <bool KnownKey>
void loadRawFromTimingData(const std::filesystem::path &directory,
                           Old::TimingData<KnownKey> &timingData);

} // namespace SaveLoad

#endif // CRYPTOLYSER_ATTACKER_SAVELOAD_HPP
