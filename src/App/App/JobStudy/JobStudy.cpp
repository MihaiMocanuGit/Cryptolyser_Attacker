#include "JobStudy.hpp"

App::JobStudy::JobStudy(const App::BuffersStudy &buffers, const std::atomic_flag &continueRunning)
    : JobI {continueRunning}
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
    input.knownKey = buffers.knownKey;
    input.key = buffers.key;
    input.packetCount = buffers.packetCount;
    input.savePath = buffers.savePath;
}

void App::JobStudy::operator()()
{
    if (input.knownKey)
    {
        ServerConnection<true> connection {input.ip, input.port};
        TimingData<true, SampleData<double>> timingData {input.dataSize, input.key};
        Gatherer<true> gatherer {std::move(connection), std::move(timingData)};

        Study<true> study {std::move(gatherer), m_continueRunning, input.savePath};
        DistributionData<double>::Bounds bounds {input.lb, input.ub};
        if (input.calibrate)
            bounds = study.calibrateBounds(1'000'000, input.lbConfidence, input.ubConfidence);
        study.run(input.packetCount, 1024 * 1024, 1024 * 1024, bounds.lb, bounds.ub);
    }
    else
    {
        ServerConnection<false> connection {input.ip, input.port};
        TimingData<false, SampleData<double>> timingData {input.dataSize};
        Gatherer<false> gatherer {std::move(connection), std::move(timingData)};

        Study<false> study {std::move(gatherer), m_continueRunning, input.savePath};
        DistributionData<double>::Bounds bounds {input.lb, input.ub};
        if (input.calibrate)
            bounds = study.calibrateBounds(1'000'000, input.lbConfidence, input.ubConfidence);
        study.run(input.packetCount, 1024, 1024 * 1024, bounds.lb, bounds.ub);
    }
}

std::string App::JobStudy::description() const noexcept
{
    std::string hexKeyArray {""};
    if (input.knownKey)
        for (std::byte byte : input.key)
            hexKeyArray += std::format("{:02x}", static_cast<unsigned>(byte));
    else
        hexKeyArray = "Unknown";
    return std::format("Study - PacketCount: {}, Key: {}, SavePath: {}", input.packetCount,
                       hexKeyArray, input.savePath.string());
}
