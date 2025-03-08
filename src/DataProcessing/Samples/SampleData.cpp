#include "SampleData.hpp"

#include <array>
#include <cmath>
#include <cstdint>

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
SampleData<Real_t>::SampleData(const std::vector<Real_t> &data)
    : m_data{data}, m_currentMetrics{SampleMetrics<Real_t>::compute(m_data.begin(), m_data.end())}
{
}

template <typename Real_t>
SampleData<Real_t>::SampleData(std::vector<Real_t> &&data)
    : m_data{std::move(data)},
      m_currentMetrics{SampleMetrics<Real_t>::compute(m_data.begin(), m_data.end())}
{
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

template <typename Real_t>
const std::vector<Real_t> &SampleData<Real_t>::data() const noexcept
{
    return m_data;
}

template class SampleData<int>;
template class SampleData<uint64_t>;
template class SampleData<float>;
template class SampleData<double>;
template class SampleData<long double>;
