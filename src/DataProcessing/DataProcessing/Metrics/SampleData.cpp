#include "SampleData.hpp"

#include <array>
#include <cmath>
#include <cstdint>

namespace Old
{
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
    : m_data{data}, m_currentMetrics{Metrics<Real_t>::compute(m_data.begin(), m_data.end())}
{
}

template <typename Real_t>
SampleData<Real_t>::SampleData(std::vector<Real_t> &&data)
    : m_data{std::move(data)},
      m_currentMetrics{Metrics<Real_t>::compute(m_data.begin(), m_data.end())}
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
const Metrics<Real_t> &SampleData<Real_t>::metrics() const noexcept
{
    return m_currentMetrics;
}

template <typename Real_t>
const std::vector<Real_t> &SampleData<Real_t>::data() const noexcept
{
    return m_data;
}

template <typename Real_t>
size_t SampleData<Real_t>::size() const noexcept
{
    return m_data.size();
}

template <typename Real_t>
const Real_t &SampleData<Real_t>::operator[](size_t index) const
{
    return m_data[index];
}

template class SampleData<int>;
template class SampleData<uint64_t>;
template class SampleData<float>;
template class SampleData<double>;
template class SampleData<long double>;
} // namespace Old
