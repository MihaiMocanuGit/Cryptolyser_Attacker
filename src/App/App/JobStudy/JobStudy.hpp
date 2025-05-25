#pragma once

#include "../JobI/JobI.hpp"
#include "Cryptolyser_Common/connection_data_types.h"
#include "ServerConnection/ServerConnection.hpp"
#include "Study/Study.hpp"

#include <array>
#include <filesystem>
#include <format>
#include <string>

namespace App
{
struct BuffersStudy
{
    uint8_t ip[4] {127, 0, 0, 1};
    uint16_t port {8081};
    bool knownKey {false};
    std::array<std::byte, PACKET_KEY_BYTE_SIZE> key {static_cast<std::byte>(0)};
    std::string savePath {""};
    size_t packetCount {1 << 26};
    bool calibrate {true};
    float lbConfidence {0.0001}, ubConfidence {0.0025};
    float lb {500}, ub {4000};
    uint32_t dataSize {16};
};

class JobStudy : public JobI
{
  private:
    struct Input
    {
        std::string ip {};
        uint16_t port {};
        bool knownKey {false};
        std::array<std::byte, PACKET_KEY_BYTE_SIZE> key {static_cast<std::byte>(0)};
        std::filesystem::path savePath {};
        size_t packetCount {};
        bool calibrate {};
        double lbConfidence {}, ubConfidence {};
        double lb {}, ub {};
        unsigned dataSize {};
    } input;

  public:
    explicit JobStudy(const BuffersStudy &buffers, const std::atomic_bool &continueRunning);

    void operator()() override;

    [[nodiscard]] std::string description() const noexcept override;

    [[nodiscard]] std::unique_ptr<JobI> clone() const override;

    ~JobStudy() override = default;
};
} // namespace App
