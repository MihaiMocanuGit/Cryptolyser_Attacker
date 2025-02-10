#include "SampleData.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <numeric>

template <typename Real_t>
SampleMetrics<Real_t> SampleMetrics<Real_t>::combineMetrics(const SampleMetrics &metric1,
                                                            const SampleMetrics &metric2) noexcept
{
    if (metric1.size == 0)
        return metric2;
    if (metric2.size == 0)
        return metric1;

    Real_t sum = metric1.sum + metric2.sum;
    size_t size = metric1.size + metric2.size;
    Real_t mean = (static_cast<Real_t>(metric1.size) * metric1.mean +
                   static_cast<Real_t>(metric2.size) * metric2.mean) /
                  static_cast<Real_t>(metric1.size + metric2.size);
    // https://math.stackexchange.com/questions/2971315/how-do-i-combine-standard-deviations-of-two-groups
    Real_t variance = (static_cast<Real_t>(metric1.size - 1) * metric1.variance +
                       static_cast<Real_t>(metric2.size - 1) * metric2.variance) /
                      static_cast<Real_t>(metric1.size + metric2.size - 1);
    variance += static_cast<Real_t>(metric1.size * metric2.size) * (metric1.mean - metric2.mean) *
                (metric1.mean - metric2.mean) / (metric1.size + metric2.size);
    Real_t stdDev = std::sqrt(variance);

    return {sum, size, mean, variance, stdDev};
}

template <typename Real_t>
void SampleData<Real_t>::m_updateMetrics(const Real_t &dataValue)
{
    std::array<Real_t, 1> tmp = {dataValue};
    m_updateMetrics(tmp.begin(), tmp.end());
}

template <typename Real_t>
void SampleData<Real_t>::reserve(std::size_t newCapacity)
{
    m_data.reserve(newCapacity);
}

template <typename Real_t>
SampleData<Real_t>::SampleData(const std::vector<Real_t> &data) : m_data{data}
{
    m_currentMetrics = SampleMetrics<Real_t>{m_data.begin(), m_data.end()};
}

template <typename Real_t>
SampleData<Real_t>::SampleData(std::vector<Real_t> &&data) : m_data{std::move(data)}
{
    m_currentMetrics = SampleMetrics<Real_t>{m_data.begin(), m_data.end()};
}

template <typename Real_t>
typename std::vector<Real_t>::const_iterator SampleData<Real_t>::insert(const Real_t &dataValue)
{
    m_updateMetrics(dataValue);
    return m_data.insert(m_data.end(), dataValue);
}

template <typename Real_t>
typename std::vector<Real_t>::const_iterator SampleData<Real_t>::begin() const noexcept
{
    return m_data.begin();
}

template <typename Real_t>
typename std::vector<Real_t>::const_iterator SampleData<Real_t>::end() const noexcept
{
    return m_data.end();
}

template <typename Real_t>
const SampleMetrics<Real_t> &SampleData<Real_t>::metrics() const noexcept
{
    return m_currentMetrics;
}

template class SampleMetrics<int>;
template class SampleMetrics<uint64_t>;
template class SampleMetrics<float>;
template class SampleMetrics<double>;
template class SampleMetrics<long double>;

template class SampleData<int>;
template class SampleData<uint64_t>;
template class SampleData<float>;
template class SampleData<double>;
template class SampleData<long double>;
