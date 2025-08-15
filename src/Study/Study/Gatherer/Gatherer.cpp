#include "Gatherer.hpp"

#include "DataProcessing/Timings/TimingProcessing.hpp"
#include "Study/TimingData/TimingData.hpp"

#include <random>

template <bool KnownKey>
std::vector<std::byte> Gatherer<KnownKey>::m_constructRandomVector()
{
    static std::mt19937 gen(0);
    static std::uniform_int_distribution<uint8_t> uniform_dist(0, 255);
    std::vector<std::byte> randomized;
    randomized.reserve(m_timingData.dataSize());
    for (size_t i {0}; i < m_timingData.dataSize(); ++i)
        randomized.push_back(static_cast<std::byte>(uniform_dist(gen)));
    return randomized;
}

template <bool KnownKey>
Gatherer<KnownKey>::ObtainStatus Gatherer<KnownKey>::obtain(uint32_t id)
{
    std::vector<std::byte> studyData {m_constructRandomVector()};
    std::optional<connection_response_t> result;
    if constexpr (KnownKey)
        result = m_connection.transmit(id, m_timingData.key(), studyData);
    else
        result = m_connection.transmit(id, studyData);

    if (not result)
    {
        m_lostPackages++;
        // Resetting the connection
        m_connection.closeConnection();
        m_connection.connect();
        return ObtainStatus::lost;
    }
    const double timing {TimingProcessing::computeDT<double>(
        result->inbound_t1, result->inbound_t2, result->outbound_t1, result->outbound_t2)};
    if (timing < m_lb)
    {
        m_sampleLB.insert(timing);
        return ObtainStatus::ignoredLB;
    }
    else if (timing > m_ub)
    {
        m_sampleUB.insert(timing);
        return ObtainStatus::ignoredUB;
    }

    for (unsigned byte {0}; byte < PACKET_AES_BLOCK_SIZE; ++byte)
    {
        size_t byteValue;
        switch (m_aesType)
        {
            case packet_type_e::ECB:
                byteValue = static_cast<size_t>(studyData[byte]);
                break;
            case packet_type_e::CBC:
                byteValue = static_cast<size_t>(studyData[byte]) ^ result->iv[byte];
                break;
            case packet_type_e::CTR:
                byteValue = result->iv[byte];
                break;
            default:
                break;
        }
        m_timingData.timing().update(byte,
                                     [timing, byteValue](size_t, auto &byte)
                                     {
                                         byte.update(byteValue, [timing](size_t, auto &byteValue)
                                                     { byteValue.insert(timing); });
                                     });
    }
    return ObtainStatus::success;
}

template <bool KnownKey>
void Gatherer<KnownKey>::init(double lb, double ub)
{
    m_lb = lb;
    m_sampleLB = {};
    m_ub = ub;
    m_sampleUB = {};

    m_lostPackages = 0;
    m_connection.connect();
}

template <bool KnownKey>
Gatherer<KnownKey>::BorrowedData Gatherer<KnownKey>::release()
{
    m_connection.closeConnection();
    return {.connection {std::move(m_connection)}, .timingData {std::move(m_timingData)}};
}

template <bool KnownKey>
packet_type_e Gatherer<KnownKey>::aesType() const noexcept
{
    return m_aesType;
}

template <bool KnownKey>
ServerConnection<KnownKey> &Gatherer<KnownKey>::connection()
{
    return m_connection;
}

template <bool KnownKey>
const TimingData<KnownKey> &Gatherer<KnownKey>::timingData() const
{
    return m_timingData;
}

template <bool KnownKey>
size_t Gatherer<KnownKey>::validValuesCount() const
{
    return m_timingData[0].globalMetric().size;
}

template <bool KnownKey>
size_t Gatherer<KnownKey>::lostPackages() const
{
    return m_lostPackages;
}

template <bool KnownKey>
const SampleData<double> &Gatherer<KnownKey>::sampleUB() const
{
    return m_sampleUB;
}

template <bool KnownKey>
double Gatherer<KnownKey>::ub() const
{
    return m_ub;
}

template <bool KnownKey>
const SampleData<double> &Gatherer<KnownKey>::sampleLB() const
{
    return m_sampleLB;
}

template <bool KnownKey>
double Gatherer<KnownKey>::lb() const
{
    return m_lb;
}

template <bool KnownKey>
Gatherer<KnownKey>::Gatherer(ServerConnection<KnownKey> &&connection,
                             TimingData<KnownKey> &&timingData, packet_type_e aesType)
    : m_connection {std::move(connection)}, m_timingData {std::move(timingData)},
      m_aesType {aesType}
{
}

template class Gatherer<true>;
template class Gatherer<false>;
