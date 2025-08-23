#include "JobStudy.hpp"

namespace App
{
JobStudy::JobStudy(const JobStudy::Buffers &buffers) : JobI {}
{
    input.calibrate = buffers.calibrate;
    input.lb = buffers.lb;
    input.ub = buffers.ub;
    input.lbConfidence = buffers.lbConfidence;
    input.ubConfidence = buffers.ubConfidence;
    input.dataSize = buffers.dataSize;
    const auto &ip = buffers.ip;
    input.ip = std::format("{}.{}.{}.{}", ip[0], ip[1], ip[2], ip[3]);
    input.port = buffers.port;
    input.aesType = static_cast<packet_type_e>(buffers.aesTypeIndex);
    input.knownKey = buffers.knownKey;
    input.key = buffers.key;
    input.packetCount = buffers.packetCount;
    input.savePath = buffers.savePath;
}

[[nodiscard]] JobStudy::ExitStatus_e JobStudy::invoke(std::stop_token stoken)
{
    if (stoken.stop_requested())
        return ExitStatus_e::KILLED;
    std::cout << "Started Study job.\n";
    if (input.knownKey)
    {
        ServerConnection<true> connection {input.ip, input.port, input.aesType};
        TimingData<true, SampleData<double>> timingData {input.dataSize, input.key};
        Gatherer<true> gatherer {std::move(connection), std::move(timingData), input.aesType};
        Study<true> study {std::move(gatherer), m_continueRunning, input.savePath};

        DistributionData<double>::Bounds bounds {input.lb, input.ub};
        if (input.calibrate)
        {
            if (stoken.stop_requested())
                return ExitStatus_e::KILLED;
            std::cout << "Started calibration...\n";
            bounds = study.calibrateBounds(1'000'000, input.lbConfidence, input.ubConfidence);
            std::cout << "Finished calibration.\n";
        }

        if (stoken.stop_requested())
            return ExitStatus_e::KILLED;
        study.run(input.packetCount, 1024 * 1024, 16 * 1024 * 1024, bounds.lb, bounds.ub);
    }
    else
    {
        ServerConnection<false> connection {input.ip, input.port, input.aesType};
        TimingData<false, SampleData<double>> timingData {input.dataSize};
        Gatherer<false> gatherer {std::move(connection), std::move(timingData), input.aesType};
        Study<false> study {std::move(gatherer), m_continueRunning, input.savePath};

        DistributionData<double>::Bounds bounds {input.lb, input.ub};
        if (input.calibrate)
        {
            if (stoken.stop_requested())
                return ExitStatus_e::KILLED;
            std::cout << "Started calibration...\n";
            bounds = study.calibrateBounds(1'000'000, input.lbConfidence, input.ubConfidence);
            std::cout << "Finished calibration.\n";
        }

        if (stoken.stop_requested())
            return ExitStatus_e::KILLED;
        study.run(input.packetCount, 1024 * 1024, 16 * 1024 * 1024, bounds.lb, bounds.ub);
    }
    std::cout << "Finished Study job.\n\n";
    return ExitStatus_e::OK;
}

std::string JobStudy::description() const noexcept
{
    std::string hexKeyArray {""};
    if (input.knownKey)
    {
        for (std::byte byte : input.key)
            hexKeyArray += std::format("{:02x} ", static_cast<unsigned>(byte));
        hexKeyArray.pop_back();
    }
    else
        hexKeyArray = "Unknown";
    return std::format("Study - AES: {}, PacketCount: {}, Key: {}, SavePath: {}",
                       packet_type_names[static_cast<uint8_t>(input.aesType)].data(),
                       input.packetCount, hexKeyArray, input.savePath.string());
}

std::unique_ptr<JobI> JobStudy::clone() const { return std::make_unique<JobStudy>(*this); }

} // namespace App
