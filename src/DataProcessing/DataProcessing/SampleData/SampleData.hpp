#ifndef CRYPTOLYSER_ATTACKER_NEW_SAMPLEDATA_HPP
#define CRYPTOLYSER_ATTACKER_NEW_SAMPLEDATA_HPP

#include "../Metrics/Metrics.hpp"
#include "../MetricsData/MetricsData.hpp"

#include <cassert>
#include <vector>

template <Real R>
class SampleData
{
  private:
    std::vector<R> m_data {};
    MetricsData<R> m_metricsData {};

  public:
    SampleData() = default;

    explicit SampleData(std::vector<R> data);

    template <typename InputIterator>
    SampleData(InputIterator begin, InputIterator end);

    void reserve(std::size_t newCapacity);

    typename std::vector<R>::const_iterator insert(R dataValue);

    typename std::vector<R>::const_iterator insert(const std::vector<R> &data);

    template <typename InputIterator>
    typename std::vector<R>::const_iterator insert(InputIterator begin, InputIterator end);

    typename std::vector<R>::const_iterator cbegin() const noexcept;

    typename std::vector<R>::const_iterator begin() const noexcept;

    typename std::vector<R>::const_iterator cend() const noexcept;

    typename std::vector<R>::const_iterator end() const noexcept;

    [[nodiscard]] const Metrics<double> &globalMetric() const noexcept;

    const std::vector<R> &data() const noexcept;

    [[nodiscard]] size_t size() const noexcept;

    const R &operator[](size_t index) const;
};

template <Real R>
const R &SampleData<R>::operator[](size_t index) const
{
    assert(index < m_data.size());
    return m_data[index];
}

template <Real R>
size_t SampleData<R>::size() const noexcept
{
    return m_metricsData.size();
}

template <Real R>
const std::vector<R> &SampleData<R>::data() const noexcept
{
    return m_data;
}

template <Real R>
const Metrics<double> &SampleData<R>::globalMetric() const noexcept
{
    return m_metricsData.globalMetric();
}

template <Real R>
typename std::vector<R>::const_iterator SampleData<R>::end() const noexcept
{
    return m_data.end();
}

template <Real R>
typename std::vector<R>::const_iterator SampleData<R>::cend() const noexcept
{
    return m_data.cend();
}

template <Real R>
typename std::vector<R>::const_iterator SampleData<R>::begin() const noexcept
{
    return m_data.begin();
}

template <Real R>
typename std::vector<R>::const_iterator SampleData<R>::cbegin() const noexcept
{
    return m_data.cbegin();
}

template <Real R>
template <typename InputIterator>
typename std::vector<R>::const_iterator SampleData<R>::insert(InputIterator begin,
                                                              InputIterator end)
{
    m_metricsData.insert(begin, end);
    return m_data.insert(m_data.end(), begin, end);
}

template <Real R>
typename std::vector<R>::const_iterator SampleData<R>::insert(R dataValue)
{
    m_metricsData.insert(dataValue);
    return m_data.insert(m_data.end(), dataValue);
}

template <Real R>
typename std::vector<R>::const_iterator SampleData<R>::insert(const std::vector<R> &data)
{
    return insert(data.begin(), data.end());
}

template <Real R>
void SampleData<R>::reserve(std::size_t newCapacity)
{
    m_data.reserve(newCapacity);
}

template <Real R>
template <typename InputIterator>
SampleData<R>::SampleData(InputIterator begin, InputIterator end)
    : m_data {begin, end}, m_metricsData {begin, end}
{
}

template <Real R>
SampleData<R>::SampleData(std::vector<R> data) : m_data {std::move(data)}, m_metricsData {m_data}
{
}
#endif // CRYPTOLYSER_ATTACKER_NEW_SAMPLEDATA_HPP
