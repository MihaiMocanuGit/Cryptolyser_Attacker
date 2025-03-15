#ifndef CRYPTOLYSER_ATTACKER_STUDY_DOPPELGANGER_HPP
#define CRYPTOLYSER_ATTACKER_STUDY_DOPPELGANGER_HPP

#include "DataProcessing/Samples/SampleGroup.hpp"
#include "ServerConnection/ServerKeyConnection.hpp"

#include <csignal>
#include <fstream>
#include <random>

// For now, this is a raw copy paste from Study that is manually modified to study the Doppelganger
// instead of the Victim. It's used in this form to easily experiment with different configurations.
// In the end, this will be refactored into a nicer interface-implementation structure
class StudyDoppelganger
{
  public:
    struct DisplayParams
    {
        size_t printFreq;
        size_t saveFreq;
        std::string savePath;
    };

    struct TimingBoundaryParams
    {
        double lb;
        double ub;
    };
    static constexpr unsigned AES_BLOCK_SIZE{16};
    static constexpr unsigned SAMPLE_GROUP_SIZE{256};

    StudyDoppelganger(ServerKeyConnection &&connection,
                      const volatile sig_atomic_t &continueRunningFlag, size_t dataPacketLength);
    StudyDoppelganger(const StudyDoppelganger &) = delete;
    StudyDoppelganger &operator=(const StudyDoppelganger &) = delete;

    void start(size_t desiredAvgSampleSize, const DisplayParams &display,
               const TimingBoundaryParams &bounds);

    [[nodiscard]] TimingBoundaryParams calibrate(const std::string &saveTo,
                                                 size_t transmissionsCount = 100'000,
                                                 double confidenceLB = 0.000025,
                                                 double confidenceUB = 0.0005);

    [[nodiscard]] const std::vector<SampleGroup<double>> &data() const;

    void loadRawStudyData(const std::string &rawDataDir);

    static void saveDataRaw(const std::string &directory,
                            const std::vector<SampleGroup<double>> &data);

    static void saveDataMetrics(const std::string &directory,
                                const std::vector<SampleGroup<double>> &data);

  private:
    const size_t m_DATA_PACKET_LENGTH;
    ServerKeyConnection m_connection;
    const volatile sig_atomic_t &m_continueRunning;
    std::vector<SampleGroup<double>> m_sampleGroups{AES_BLOCK_SIZE,
                                                    SampleGroup<double>{SAMPLE_GROUP_SIZE}};
    struct StudyContext;
    size_t m_totalCount(const StudyContext &ctx) const;
    bool filterCurrentValue(StudyContext &ctx, double timing);
    void printStats(StudyContext &ctx);
    bool isSaveTime(const StudyContext &ctx) const;
};

#endif // CRYPTOLYSER_ATTACKER_STUDY_DOPPELGANGER_HPP
