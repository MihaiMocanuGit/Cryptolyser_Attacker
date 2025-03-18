#ifndef CRYPTOLYSER_ATTACKER_SAVELOAD_HPP
#define CRYPTOLYSER_ATTACKER_SAVELOAD_HPP
#include "DataProcessing/Samples/SampleGroup.hpp"
#include "Study/TimingData/TimingData.hpp"

#include <filesystem>

namespace SaveLoad
{
// METRICS SAMPLE GROUP
void saveMetricsFromSampleGroup(const std::filesystem::path &filename,
                                const SampleGroup<double> &sampleGroup);

// METRICS TIMING DATA
template <bool KnownKey>
void saveMetricsFromTimingData(const std::filesystem::path &directory,
                               const TimingData<KnownKey> &timingData);

// RAW SAMPLE DATA
void saveRawFromSampleData(const std::filesystem::path &filename,
                           const SampleData<double> &sampleData);

void loadRawFromSampleData(const std::filesystem::path &filename, SampleGroup<double> &sampleData);

// RAW SAMPLE GROUP
void saveRawFromSampleGroup(const std::filesystem::path &directory,
                            const SampleGroup<double> &sampleGroup);

void loadRawFromSampleGroup(const std::filesystem::path &directory,
                            SampleGroup<double> &sampleGroup);

// RAW TIMING DATA
template <bool KnownKey>
void saveRawFromTimingData(const std::filesystem::path &directory,
                           const TimingData<KnownKey> &timingData);
template <bool KnownKey>
void loadRawFromTimingData(const std::filesystem::path &directory,
                           TimingData<KnownKey> &timingData);

} // namespace SaveLoad

#endif // CRYPTOLYSER_ATTACKER_SAVELOAD_HPP
