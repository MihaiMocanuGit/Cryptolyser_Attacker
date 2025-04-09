#ifndef CRYPTOLYSER_ATTACKER_NEW_SAMPLEDATA_HPP
#define CRYPTOLYSER_ATTACKER_NEW_SAMPLEDATA_HPP

#include "../MetricsData/MetricsData.hpp"
#include "../Samples/SampleMetrics.hpp"

#include <cassert>
#include <vector>

namespace New
{
template <Real T>
class SampleData
{
  private:
    std::vector<T> m_data{};
    MetricsData<T> m_metricsData{};

  public:
    SampleData() = default;

    explicit SampleData(std::vector<T> data);

    template <typename InputIterator>
    SampleData(InputIterator begin, InputIterator end);

    void reserve(std::size_t newCapacity);

    typename std::vector<T>::const_iterator insert(T dataValue);

    typename std::vector<T>::const_iterator insert(const std::vector<T> &data);

    template <typename InputIterator>
    typename std::vector<T>::const_iterator insert(InputIterator begin, InputIterator end);

    typename std::vector<T>::const_iterator cbegin() const noexcept;

    typename std::vector<T>::const_iterator begin() const noexcept;

    typename std::vector<T>::const_iterator cend() const noexcept;

    typename std::vector<T>::const_iterator end() const noexcept;

    [[nodiscard]] const SampleMetrics<double> &globalMetric() const noexcept;

    const std::vector<T> &data() const noexcept;

    [[nodiscard]] size_t size() const noexcept;

    const T &operator[](size_t index) const;
};

template <Real T>
const T &SampleData<T>::operator[](size_t index) const
{
    assert(index < m_data.size());
    return m_data[index];
}

template <Real T>
size_t SampleData<T>::size() const noexcept
{
    return m_metricsData.size();
}

template <Real T>
const std::vector<T> &SampleData<T>::data() const noexcept
{
    return m_data;
}

template <Real T>
const SampleMetrics<double> &SampleData<T>::globalMetric() const noexcept
{
    return m_metricsData.globalMetric();
}

template <Real T>
typename std::vector<T>::const_iterator SampleData<T>::end() const noexcept
{
    return m_data.end();
}

template <Real T>
typename std::vector<T>::const_iterator SampleData<T>::cend() const noexcept
{
    return m_data.cend();
}

template <Real T>
typename std::vector<T>::const_iterator SampleData<T>::begin() const noexcept
{
    return m_data.begin();
}

template <Real T>
typename std::vector<T>::const_iterator SampleData<T>::cbegin() const noexcept
{
    return m_data.cbegin();
}

template <Real T>
template <typename InputIterator>
typename std::vector<T>::const_iterator SampleData<T>::insert(InputIterator begin,
                                                              InputIterator end)
{
    m_metricsData.insert(begin, end);
    return m_data.insert(m_data.end(), begin, end);
}

template <Real T>
typename std::vector<T>::const_iterator SampleData<T>::insert(T dataValue)
{
    m_metricsData.insert(dataValue);
    return m_data.insert(m_data.end(), dataValue);
}

template <Real T>
typename std::vector<T>::const_iterator SampleData<T>::insert(const std::vector<T> &data)
{
    return insert(data.begin(), data.end());
}

template <Real T>
void SampleData<T>::reserve(std::size_t newCapacity)
{
    m_data.reserve(newCapacity);
}

template <Real T>
template <typename InputIterator>
SampleData<T>::SampleData(InputIterator begin, InputIterator end)
    : m_data{begin, end}, m_metricsData{begin, end}
{
}

template <Real T>
SampleData<T>::SampleData(std::vector<T> data) : m_data{std::move(data)}, m_metricsData{m_data}
{
}
} // namespace New
#endif // CRYPTOLYSER_ATTACKER_NEW_SAMPLEDATA_HPP
