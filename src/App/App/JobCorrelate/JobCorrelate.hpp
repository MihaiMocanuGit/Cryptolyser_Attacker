#pragma once
#include "../JobI/JobI.hpp"
#include "Cryptolyser_Common/connection_data_types.h"

#include <array>
#include <filesystem>
#include <string>
#include <vector>

namespace App
{
struct BuffersCorrelate
{
    std::vector<std::string> doppelLoadPaths {};
    std::vector<std::string> victimLoadPaths {};
    bool victimKeyKnown {true};
    std::array<std::byte, PACKET_KEY_BYTE_SIZE> victimKey {std::byte(0)};
    std::string savePath {};
};

class JobCorrelate : public JobI
{
  private:
    struct Input
    {
        std::vector<std::filesystem::path> doppelLoadPaths {};
        std::vector<std::filesystem::path> victimLoadPaths {};
        bool victimKeyKnown {true};
        std::filesystem::path savePath {};
        std::array<std::byte, PACKET_KEY_BYTE_SIZE> victimKey {std::byte(0)};
    } input;

  public:
    explicit JobCorrelate(const BuffersCorrelate &buffers, const std::atomic_bool &continueRunning);

    void operator()() override;

    [[nodiscard]] std::string description() const noexcept override;

    [[nodiscard]] std::unique_ptr<JobI> clone() const override;

    ~JobCorrelate() override = default;
};
} // namespace App
