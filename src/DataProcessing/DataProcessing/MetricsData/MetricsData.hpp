#ifndef CRYPTOLYSER_ATTACKER_METRICSDATA_HPP
#define CRYPTOLYSER_ATTACKER_METRICSDATA_HPP

#include "../Samples/SampleMetrics.hpp"

#include <concepts>
#include <vector>

template <Real T>
class MetricsData
{
  private:
    SampleMetrics<T> m_globalMetrics{};

    template <typename InputIterator>
    void m_updateMetrics(InputIterator begin, InputIterator end);

    void m_updateMetrics(T dataValue) noexcept;

    void m_updateMetrics(const SampleMetrics<T> &dataMetric) noexcept;

  public:
    MetricsData() noexcept = default;

    explicit MetricsData(const SampleMetrics<T> &metric) noexcept;

    explicit MetricsData(const std::vector<T> &data) noexcept;

    template <typename InputIterator>
    MetricsData(InputIterator begin, InputIterator end);

    void insert(T dataValue) noexcept;

    void insert(const SampleMetrics<T> &metric) noexcept;

    void insert(const std::vector<T> &data);

    template <typename InputIterator>
    void insert(InputIterator begin, InputIterator end);

    const SampleMetrics<T> &globalMetric() const noexcept;

    [[nodiscard]] size_t size() const noexcept;

    // Structural function, doesn't do anything
    inline void reserve(size_t) const noexcept;
};

template <Real T>
void MetricsData<T>::reserve(size_t) const noexcept
{
}

template <Real T>
void MetricsData<T>::insert(const std::vector<T> &data)
{
    insert(data.begin(), data.end());
}

template <Real T>
size_t MetricsData<T>::size() const noexcept
{
    return m_globalMetrics.size;
}

template <Real T>
void MetricsData<T>::insert(const SampleMetrics<T> &metric) noexcept
{
    m_updateMetrics(metric);
}

template <Real T>
void MetricsData<T>::insert(T dataValue) noexcept
{
    m_updateMetrics(dataValue);
}

template <Real T>
template <typename InputIterator>
void MetricsData<T>::insert(InputIterator begin, InputIterator end)
{
    m_updateMetrics(begin, end);
}

template <Real T>
const SampleMetrics<T> &MetricsData<T>::globalMetric() const noexcept
{
    return m_globalMetrics;
}

template <Real T>
template <typename InputIterator>
MetricsData<T>::MetricsData(InputIterator begin, InputIterator end)
    : MetricsData(SampleMetrics<T>::compute(begin, end))
{
}

template <Real T>
MetricsData<T>::MetricsData(const SampleMetrics<T> &metric) noexcept : m_globalMetrics{metric}
{
}

template <Real T>
MetricsData<T>::MetricsData(const std::vector<T> &data) noexcept
    : MetricsData(data.begin(), data.end())
{
}

template <Real T>
void MetricsData<T>::m_updateMetrics(T dataValue) noexcept
{
    std::array<T, 1> tmp = {dataValue};
    m_updateMetrics(tmp.begin(), tmp.end());
}

template <Real T>
template <typename InputIterator>
void MetricsData<T>::m_updateMetrics(InputIterator begin, InputIterator end)
{
    SampleMetrics<T> newMetrics = SampleMetrics<T>::compute(begin, end);
    m_updateMetrics(newMetrics);
}

template <Real T>
void MetricsData<T>::m_updateMetrics(const SampleMetrics<T> &dataMetric) noexcept
{
    m_globalMetrics = SampleMetrics<T>::combineMetrics(m_globalMetrics, dataMetric);
}

#endif // CRYPTOLYSER_ATTACKER_METRICSDATA_HPP
