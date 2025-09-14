#pragma once
#include "../JobI/JobI.hpp"
#include "Correlate/Correlate.hpp"
#include "Cryptolyser_Common/connection_data_types.h"
#include "DataProcessing/MetricsData/MetricsData.hpp"

#include <array>
#include <filesystem>
#include <string>
#include <vector>

namespace App
{

class JobCorrelate : public JobI
{
  private:
    struct Input
    {
        struct Group
        {
            std::vector<std::filesystem::path> doppelLoadPaths {};
            std::vector<std::filesystem::path> victimLoadPaths {};
        };
        std::vector<Group> groups;
        bool victimKeyKnown {true};
        std::filesystem::path savePath {};
        std::array<std::byte, PACKET_KEY_SIZE> victimKey {std::byte(0)};
    } input;

    Correlate<MetricsData<double>, MetricsData<double>> m_computeCorrelation() const;

    std::string m_createCorrelationDataString(
        const Correlate<MetricsData<double>, MetricsData<double>> &correlate,
        std::array<unsigned, PACKET_AES_BLOCK_SIZE> &byteCorrPos) const;

    std::string
        m_summariseKeyStats(const Correlate<MetricsData<double>, MetricsData<double>> &correlate,
                            const std::array<unsigned, PACKET_AES_BLOCK_SIZE> &byteCorrPos) const;

  public:
    struct Buffers
    {
        struct Group
        {
            std::vector<std::string> doppelLoadPaths {};
            std::vector<std::string> victimLoadPaths {};
        };
        std::vector<Group> groups;
        bool victimKeyKnown {true};
        std::array<std::byte, PACKET_KEY_SIZE> victimKey {std::byte(0)};
        std::string savePath {};
    };

    explicit JobCorrelate(const Buffers &buffers, const std::atomic_bool &continueRunning);

    void operator()() override;

    [[nodiscard]] std::string description() const noexcept override;

    [[nodiscard]] std::unique_ptr<JobI> clone() const override;

    ~JobCorrelate() override = default;
};
} // namespace App
