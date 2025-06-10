#ifndef CRYPTOLYSER_ATTACKER_CORRELATE_HPP
#define CRYPTOLYSER_ATTACKER_CORRELATE_HPP

#include "Study/TimingData/TimingData.hpp"

#include <vector>

template <HasMetric DataTypeVictim, HasMetric DataTypeDoppel>
class Correlate
{
  private:
    std::vector<std::array<double, 256>> m_correlation {PACKET_KEY_BYTE_SIZE, {0.0}};

    static void m_normalize(std::array<double, 256> &correlation);

  public:
    Correlate() = default;
    Correlate(const TimingData<false, DataTypeVictim> &victimData,
              const TimingData<true, DataTypeDoppel> &doppelgangerData);

    [[nodiscard]] const std::vector<std::array<double, 256>> &data() const;
    [[nodiscard]] std::vector<std::array<std::pair<double, std::byte>, 256>> order() const;

    template <HasMetric DataTypeVictim2 = DataTypeVictim,
              HasMetric DataTypeDoppel2 = DataTypeDoppel>
    Correlate<DataTypeVictim, DataTypeDoppel> &
        operator+=(const Correlate<DataTypeVictim2, DataTypeDoppel2> &rhs);

    template <HasMetric DataTypeVictim2 = DataTypeVictim,
              HasMetric DataTypeDoppel2 = DataTypeDoppel>
    Correlate<DataTypeVictim, DataTypeDoppel>
        operator+(const Correlate<DataTypeVictim2, DataTypeDoppel2> &rhs) const;
};

template <HasMetric DataTypeVictim, HasMetric DataTypeDoppel>
template <HasMetric DataTypeVictim2, HasMetric DataTypeDoppel2>
Correlate<DataTypeVictim, DataTypeDoppel> Correlate<DataTypeVictim, DataTypeDoppel>::operator+(
    const Correlate<DataTypeVictim2, DataTypeDoppel2> &rhs) const
{
    Correlate result {*this};
    return result += rhs;
}

template <HasMetric DataTypeVictim, HasMetric DataTypeDoppel>
template <HasMetric DataTypeVictim2, HasMetric DataTypeDoppel2>
Correlate<DataTypeVictim, DataTypeDoppel> &Correlate<DataTypeVictim, DataTypeDoppel>::operator+=(
    const Correlate<DataTypeVictim2, DataTypeDoppel2> &rhs)
{
    for (unsigned byte {0}; byte < PACKET_KEY_BYTE_SIZE; ++byte)
    {
        for (unsigned value {0}; value < 256; ++value)
            m_correlation[byte][value] += rhs.data()[byte][value];
    }
    return *this;
}

template <HasMetric DataTypeVictim, HasMetric DataTypeDoppel>
std::vector<std::array<std::pair<double, std::byte>, 256>>
    Correlate<DataTypeVictim, DataTypeDoppel>::order() const
{
    std::vector<std::array<std::pair<double, std::byte>, 256>> result {PACKET_KEY_BYTE_SIZE};

    for (unsigned byte {0}; byte < PACKET_KEY_BYTE_SIZE; ++byte)
    {
        for (unsigned value {0}; value < 256; ++value)
            result[byte][value] =
                std::make_pair(m_correlation[byte][value], static_cast<std::byte>(value));

        std::sort(result[byte].begin(), result[byte].end(),
                  [](const auto &first, const auto &second) { return first.first > second.first; });
    }

    return result;
}

template <HasMetric DataTypeVictim, HasMetric DataTypeDoppel>
const std::vector<std::array<double, 256>> &Correlate<DataTypeVictim, DataTypeDoppel>::data() const
{
    return m_correlation;
}

template <HasMetric DataTypeVictim, HasMetric DataTypeDoppel>
Correlate<DataTypeVictim, DataTypeDoppel>::Correlate(
    const TimingData<false, DataTypeVictim> &victimData,
    const TimingData<true, DataTypeDoppel> &doppelgangerData)
{
    const TimingData<true, DataTypeVictim> &t {doppelgangerData};
    const TimingData<false, DataTypeDoppel> &u {victimData};

    for (unsigned byte {0}; byte < AES_BLOCK_BYTE_SIZE; ++byte)
    {
        for (unsigned i {0}; i < 256; ++i)
        {
            long double correlation_i {0.0};
            for (unsigned j {0}; j < 256; ++j)
            {
                long double meanT {t[byte].standardizeMetric(j).mean};
                long double t_j {meanT};
                t_j -= t.timing().standardizeMetric(byte).mean;

                unsigned u_index {i ^ j ^ static_cast<unsigned>(t.key()[byte])};
                long double meanU {u[byte].standardizeMetric(u_index).mean};
                long double u_i_j {meanU};
                u_i_j -= u.timing().standardizeMetric(byte).mean;

                long double value {t_j * u_i_j};
                correlation_i += value;
            }
            m_correlation[byte][i] = static_cast<double>(correlation_i);
        }
        m_normalize(m_correlation[byte]);
    }
}

template <HasMetric DataTypeVictim, HasMetric DataTypeDoppel>
void Correlate<DataTypeVictim, DataTypeDoppel>::m_normalize(std::array<double, 256> &correlation)
{
    double max {*std::max_element(correlation.begin(), correlation.end())};
    double min {*std::min_element(correlation.begin(), correlation.end())};
    double mid {(max + min) / 2.0};
    double halfLength {(max - min) / 2.0};
    if (halfLength == 0)
        halfLength = 1;

    for (unsigned value {0}; value < 256; ++value)
        correlation[value] = (correlation[value] - mid) / halfLength;
}

#endif // CRYPTOLYSER_ATTACKER_CORRELATE_HPP
