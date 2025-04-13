#ifndef CRYPTOLYSER_ATTACKER_DISTRIBUTIONBYTEVALUE_HPP
#define CRYPTOLYSER_ATTACKER_DISTRIBUTIONBYTEVALUE_HPP

#include "../Metrics/SampleData.hpp"

#include <algorithm>
#include <execution>

class DistributionByteValue
{
    size_t m_start{};
    size_t m_stop{};
    size_t m_sum{};
    Old::SampleData<size_t> m_frequency{};
    size_t m_peakIndex{};

    template <typename Real_t>
    static Old::SampleData<size_t> initFreq(size_t start, size_t stop,
                                            const Old::SampleData<Real_t> &timings);

  public:
    template <typename Real_t>
    explicit DistributionByteValue(const Old::SampleData<Real_t> &sampleData);

    struct Bounds
    {
        size_t lb, ub;
    };
    [[nodiscard]] Bounds bounds(double confidenceLB, double confidenceUB);
    [[nodiscard]] size_t start() const;
    [[nodiscard]] size_t stop() const;
    [[nodiscard]] const Old::SampleData<size_t> &frequency() const;
    [[nodiscard]] size_t peakValue() const;
};

template <typename Real_t>
DistributionByteValue::DistributionByteValue(const Old::SampleData<Real_t> &sampleData)
    : m_start{static_cast<size_t>(sampleData.metrics().min)},
      m_stop{static_cast<size_t>(sampleData.metrics().max)},
      m_sum{sampleData.data().size()}, m_frequency{initFreq(m_start, m_stop, sampleData)}
{
    const auto it = std::find(std::execution::par_unseq, m_frequency.begin(), m_frequency.end(),
                              m_frequency.metrics().max);
    m_peakIndex = std::distance(m_frequency.begin(), it);
}

template <typename Real_t>
Old::SampleData<size_t> DistributionByteValue::initFreq(size_t start, size_t stop,
                                                        const Old::SampleData<Real_t> &timings)
{
    std::vector<size_t> frequency(stop + 1 - start, 0);
    for (double data : timings)
    {
        size_t index{static_cast<size_t>(data) - start};
        frequency[index]++;
    }
    return Old::SampleData{std::move(frequency)};
}

#endif // CRYPTOLYSER_ATTACKER_DISTRIBUTIONBYTEVALUE_HPP
